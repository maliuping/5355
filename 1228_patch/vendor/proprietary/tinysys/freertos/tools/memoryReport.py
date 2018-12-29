#!/usr/bin/env python

from __future__ import print_function
import sys
import os, os.path
import re
import string
import argparse
import ConfigParser

WORKAROUND_XTENSA_LITERAL = 1

class GnuMapFile:

    def __init__(self, f):
        self.f = f
        self.l = None
        self.section_order = []
        self.sections = {}

    def get(self):
        if self.l is not None:
            l = self.l
            self.l = None
            return l
        return next(self.f)

    def unget(self, l):
        assert self.l is None
        self.l = l

    def peek(self):
        if self.l is None:
            self.l = next(self.f)
        return self.l

    def skip_till_memmap(self):
        for l in self.f:
            if l == "Linker script and memory map\n":
                break

    def skip_while_lead_space(self):
        while 1:
            if self.l is not None and self.l[0] not in [" ", "\n"]:
                break
            self.l = next(self.f)

    def parse_section_line(self):
        l = self.get().rstrip()
        print(l)
        if l[1] == " " and l[38] != " ":
            fields = l.split(None, 2)
            fields = [""] + fields
        else:
            fields = l.split()
            if " = " in l or "linker stubs" in l or "PROVIDE " in l:
                return [fields[0], 0, 0]
            if len(fields) == 1:
                # For long section names, continuation on 2nd line
                fields2 = self.get().split()
                if len(fields2) < 2:
                    return [fields[0], 0, 0]
                else:
                    fields += fields2
        return [fields[0], int(fields[1], 0), int(fields[2], 0)] + fields[3:]

    def parse_section(self):
        sec_name, addr, sz = self.parse_section_line()[0:3]
        if sz == 0:
            # logging.debug("Empty section: %s", sec_name)
            self.skip_while_lead_space()
            return

        # print(sec_name, addr, sz)
        self.section_order.append((sec_name, addr, sz))
        self.sections[sec_name] = {"addr": addr, "size": sz}

        last_obj = None
        while 1:
            l = self.get()
            if l.startswith("LOAD ") or l.startswith("START ") or l.startswith("END ") or l.startswith("OUTPUT "):
                self.unget(l)
                break

            if l == "\n":
                break

            if len(l) > 16 and l[16] == " ":
                assert "(size before relaxing)" in l
                continue

            if l[1] == " " and l[38] == " ":
                addr, sym = l.split(None, 1)
                # print((addr, sym))
                if " = " in sym or "PROVIDE " in sym or "ASSERT " in sym:
                    # special symbol, like ABSOLUTE, ALIGN, etc.
                    continue
                self.sections[sec_name]["objects"][-1][-1].append((int(addr, 0), sym.rstrip()))
                continue

            if l.startswith(" *fill* "):
                self.unget(l)
                fields = self.parse_section_line()
                if fields[2] == 0:
                    continue
                if last_obj is None:
                    name = sec_name + ".initial_align"
                else:
                    name = last_obj + ".fill"
                self.sections[sec_name].setdefault("objects", []).append(fields + [name, []])
                continue

            if l.find(".a:") != -1:
                continue

            if l.find(".o(") != -1:
                continue

            if l.startswith(" *"):
                # section group "comment"
                continue
            # print("*", l)

            self.unget(l)
            fields = self.parse_section_line()
            if fields[2] == 0:
                if WORKAROUND_XTENSA_LITERAL:
                    # Empty *.literal sub-section followed by
                    # non-empty fill is actually non-empty *.literal
                    if fields[0].endswith(".literal"):
                        if self.peek().startswith(" *fill* "):
                            fields2 = self.parse_section_line()
                            if fields2[2] != 0:
                                # assert 0, (fields, fields2)
                                fields[2] = fields2[2]
                                fields[-1] += ".literal"
                                self.sections[sec_name].setdefault("objects", []).append(fields + [[]])
                continue

            # If an absolute library name (libc, etc.), use the last component
            # if fields[-1][0] == "/":
            #     fields[-1] = fields[-1].rsplit("/", 1)[1]

            self.sections[sec_name].setdefault("objects", []).append(fields + [[]])
            last_obj = fields[-1]
            # assert 0, fields

    def parse_sections(self):
        while 1:
            l = self.get().rstrip()
            if l.startswith("LOAD ") and len(l.split()) == 2:
                continue
            elif l.startswith("START ") and len(l.split()) == 2:
                continue
            elif l.startswith("END ") and len(l.split()) == 2:
                continue
            elif l == "":
                continue
            elif l.startswith("OUTPUT"):
                break
            else:
                self.unget(l)
                self.parse_section()

    # Validate that all sub-sections are adjacent (and thus don't overlap),
    # and fill in parent section completely. Note that read-only string
    # section may be subject of string merging optimization by linker,
    # and fail adjacency check. To "fix" this, "M" flag on ELF section
    # should be unset (e.g. with objcopy --set-section-flags)
    def validate(self):
        for k, sec_addr, sec_sz in self.section_order:
            next_addr = sec_addr
            for sec, addr, sz, obj, symbols in self.sections[k]["objects"]:
                if addr != next_addr:
                    # print("%x vs %x" % (addr, next_addr))
                    next_addr = addr
                next_addr += sz
            assert next_addr <= sec_addr + sec_sz, "0x%x vs 0x%x" % (next_addr, sec_addr + sec_sz)

def parse_path(path):
    if path.find('mtkeda') != -1:
        return path
    else:
        return '/'.join(path.split('_intermediates/')[-1].split('/')[3:])

def query_feature(features, path):
    feature = 'Platform'; subFeature = ''

    if path.find('heap') != -1:
        feature = 'Heap'
    elif path.find('HEAP') != -1:
        feature = 'Heap'
    elif path.find('RTOS') != -1:
        feature = 'RTOS'
    elif path.find('mtkeda') != -1:
        feature = 'C-lib'
    else:
        for k in features.iterkeys():
            if path.find(k) != -1:
                feature = (features[k])[0]
                subFeature = (features[k])[1]
                break
    return (feature, subFeature)


def main():
    p = argparse.ArgumentParser(prog='memoryReport.py', add_help=True,
                                description="Report memory map by features")
    p.add_argument('-d', action='store_true', default=False, dest='dump', help='dump all output')
    p.add_argument('projType', help='project type', metavar='Project_Type')
    p.add_argument('iniFile', help='feature list', metavar='Feature_List')
    p.add_argument('mapFile', help='memory map', metavar='Memory_Map')
    # p.add_argument('prefix', help='prefix', metavar='Path_Prefix')
    args = vars (p.parse_args())
    dump = args['dump']
    projType = args['projType'].upper()
    iniFile = args['iniFile']
    mapFile = args['mapFile']
    # prefix = args['prefix']
    platform = os.environ.get('PLATFORM').lower()
    settings = []
    features = {}
    sectionS = set()
    featureS = set(['Platform', 'Heap', 'RTOS', 'C-lib'])
    subFeatureS = set()
    records = []
    result1 = {}
    result2 = {}
    criteria = {}
    platformSection = ''
    ret = 0

    if projType == 'SCP':
        settings = ['TinySys-COMMON', 'TinySys-SCP']

    f = open(iniFile)
    config = ConfigParser.ConfigParser()
    config.optionxform = lambda option: option
    config.readfp(f)

    for i in settings:
        for j in config.items(i):
            features[j[0]] = j[1].strip().split(':')
            featureS.add(features[j[0]][0])
            subFeatureS.add(features[j[0]][1])

    platformSection = "%s-%s" % (projType, platform)
    if config.has_section(platformSection):
        for i in config.items(platformSection):
            criteria[i[0]] = i[1]

    f.close()
    featureS = filter(None, featureS)
    subFeatureS = filter(None, subFeatureS)

    f = open(mapFile)
    m = GnuMapFile(f)
    m.skip_till_memmap()
    m.skip_while_lead_space()
    m.parse_sections()
    m.validate()

    for k, addr, sz in m.section_order:
        # print("%08x %08x %s" % (addr, sz, k))
        section = k; size = sz
        if k.find('.debug_') != -1:
            continue
        sectionS.add(k)
        for sec, addr, sz, obj, symbols in m.sections[k]['objects']:
            # print(" %08x %08x %s" % (addr, sz, obj.replace(prefix, "")))
            symbol = '.'.join((sec.split('.'))[2:])
            address = addr; size = sz
            # path = obj.replace(prefix, '')
            path = parse_path(obj)
            [feature, subFeature] = query_feature(features, path)
            if symbols and len(symbols) > 1:
                for i in range(len(symbols) -1):
                    address = symbols[i][0]; symbol = symbols[i][1]
                    # print("  %08x %s" % (address, symbol))
                    records.append([symbol, feature, subFeature, section, address, int(symbols[i+1][0]) - int(address), path])
                i += 1
                records.append([symbols[i][1], feature, subFeature, section, symbols[i][0], int(addr) + int(sz) - int(symbols[i][0]), path])
            elif len(symbols) == 1:
                i = 0
                records.append([symbols[i][1], feature, subFeature, section, symbols[i][0], size, path])
            else:
                records.append((symbol, feature, subFeature, section, address, size, path))

    f.close()

    for s in sectionS:
        for f in featureS:
            result1[(f, s)] = 0
        for f in subFeatureS:
            result2[(f, s)] = 0

    for symbol, feature, subFeature, section, address, size, path in records:
        if dump: print("%-60s\t%-9s\t%-9s\t%-13s\t0x%08x\t%s\t%s" % (symbol, feature, subFeature, section, address, size, path))
        if feature and section:
            result1[(feature, section)] += size
        if subFeature and section:
            result2[(subFeature, section)] += size

    if dump: print(); print(); print()
    if dump: print("%20s\t" % '', end='')
    for s in sorted(sectionS):
        if dump: print("%20s\t" % s, end='')
    if dump: print("%20s" % "Sum")
    for f in sorted(featureS):
        if dump: print("%20s\t" % f, end='')
        result1[(f, 'sum')] = 0
        for s in sorted(sectionS):
            v = result1.get((f, s), 0)
            result1[(f, 'sum')] += v
            if dump: print("%20d\t" % v, end='')
        if dump: print("%20d" % result1[(f, 'sum')])

    if dump: print(); print(); print()
    if dump: print("%20s\t" % '', end='')
    for s in sorted(sectionS):
        if dump: print("%20s\t" % s, end='')
    if dump: print("%20s" % "Sum")
    for f in sorted(subFeatureS):
        if dump: print("%20s\t" % f, end='')
        result2[(f, 'sum')] = 0
        for s in sorted(sectionS):
            v = result2.get((f, s), 0)
            result2[(f, 'sum')] += v
            if dump: print("%20d\t" % v, end='')
        if dump: print("%20d" % result2[(f, 'sum')])

    if config.has_section(platformSection):
        if dump: print(); print()
        print()
        for f in sorted(featureS):
            if f == 'Peripheral':
                continue
            c = int(criteria.get(f, 0))
            m = int(result1.get((f, 'sum'), 0))
            if m > c:
                print("%s: %s(%d>%d) is out of memory limitation" % (projType, f, m, c))
                ret = 13
        for f in sorted(subFeatureS):
            c = int(criteria.get(f, 0))
            m = int(result2.get((f, 'sum'), 0))
            if m > c:
                print("%s: %s(%d>%d) is out of memory limitation" % (projType, f, m, c))
                ret = 13

        if ret == 0: print("%s: pass memory limitation check" %(projType)); print()
    return ret

if '__main__'==__name__:
        ret = main()
        sys.exit(ret)

