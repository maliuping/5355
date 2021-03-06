# Define project-specific configuration options here.
#
# A configuration option is defined in the simple format:
#   <KEY> = <VALUE>
# where spaces around the = character is optional.
#
# You can define the same configuration options appeared in platform.mk with
# different values. The options defined here take higher precedence.
#
# **** DO NOT **** define anything other than configuration options here.
# If you need to customize project-specific source files, compiler flags
# or required libraries, add them to CompilerOption.mk.
CFG_FPGA_SUPPORT = yes

##############################################################################
# Support for 4GB mode, if the option CFG_MTK_DRAM_4GB_SUPPORT is set to "yes",
# when cm4 access dram space, we will offset one more gigabyte.
##############################################################################
CFG_MTK_DRAM_4GB_SUPPORT=yes
