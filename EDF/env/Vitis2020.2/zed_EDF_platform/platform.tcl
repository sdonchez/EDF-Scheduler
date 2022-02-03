# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /workspaces/Research-I/EDF/env/Vitis2020.2/zed_EDF_platform/platform.tcl
# 
# OR launch xsct and run below command.
# source /workspaces/Research-I/EDF/env/Vitis2020.2/zed_EDF_platform/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {zed_EDF_platform}\
-hw {/workspaces/Research-I/EDF/hw_zed_gpio/hw_zed_gpio.xsa}\
-proc {ps7_cortexa9} -os {linux} -fsbl-target {psu_cortexa53_0} -out {/workspaces/Research-I/EDF/env/Vitis2020.2}

platform write
platform active {zed_EDF_platform}
domain config -image {/workspaces/Research-I/EDF/zedboard-petalinux-gpio/images/linux}
platform write
domain config -rootfs {/workspaces/Research-I/EDF/zedboard-petalinux-gpio/images/linux/rootfs.tar.gz}
platform write
domain config -sysroot {/workspaces/Research-I/EDF/sdk/sysroots/x86_64-petalinux-linux}
platform write
domain config -image {/workspaces/Research-I/EDF/sdk/platform/image}
platform write
domain config -boot {/workspaces/Research-I/EDF/sdk/platform/boot}
platform write
domain config -bif {/workspaces/Research-I/EDF/sdk/platform/boot/linux.bif}
platform write
domain config -rootfs {/workspaces/Research-I/EDF/sdk/platform/image/system.dtb}
platform write
platform generate
domain config -rootfs {/workspaces/Research-I/EDF/zedboard-petalinux-gpio/images/linux/rootfs.cpio}
platform write
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
domain config -sysroot {/workspaces/Research-I/EDF/sdk/sysroots/cortexa9t2hf-neon-xilinx-linux-gnueabi/bin}
domain config -sysroot {/workspaces/Research-I/EDF/sdk/sysroots/cortexa9t2hf-neon-xilinx-linux-gnueabi/usr}
platform generate
platform active {zed_EDF_platform}
platform config -updatehw {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/project-spec/hw-description/system.xsa}
domain config -bif {}
platform write
domain config -boot {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images}
platform write
domain config -image {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images}
platform write
domain config -rootfs {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images/rootfs.cpio.gz}
platform write
domain config -sysroot {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/sdk/sysroots/x86_64-petalinux-linux}
platform write
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
domain config -sysroot {}
platform write
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
domain config -sysroot {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/sdk/sysroots/x86_64-petalinux-linux}
platform write
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains zynq_fsbl 
platform clean
platform generate
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform clean
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform clean
platform generate
platform clean
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate -domains zynq_fsbl 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {zed_EDF_platform}
platform active {zed_EDF_platform}
domain config -boot {/workspaces/Research-I/EDF/platform/boot}
platform write
platform clean
platform clean
platform generate
domain config -bif {/workspaces/Research-I/EDF/platform/ZedEDF.bif}
platform write
platform generate -domains 
platform config -remove-boot-bsp
platform write
platform generate -domains zynq_fsbl 
platform config -fsbl-elf {/workspaces/Research-I/EDF/platform/boot/zynq_fsbl.elf}
platform write
platform generate -domains 
platform generate -domains zynq_fsbl 
platform clean
platform generate
platform clean
platform clean
platform generate
platform active {zed_EDF_platform}
platform config -fsbl-elf {/workspaces/Research-I/sd/boot/zynq_fsbl.elf}
platform write
domain config -boot {/workspaces/Research-I/sd/boot}
platform write
domain config -bif {/workspaces/Research-I/EDF/env/Vitis2020.2/zed_EDF_system/Debug/system.bif}
platform write
domain config -image {/workspaces/Research-I/sd/boot}
platform write
domain config -rootfs {/workspaces/Research-I/sd/fs/rootfs.ext4}
platform write
platform generate -domains 
platform active {zed_EDF_platform}
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform clean
platform clean
platform active {zed_EDF_platform}
platform config -fsbl-elf {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images/zynq_fsbl.elf}
platform write
domain config -boot {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images}
domain config -boot {}
domain config -bif {}
domain config -boot {}
domain config -boot {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images}
platform write
domain config -image {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images}
platform write
domain config -rootfs {/workspaces/Research-I/EDF/avnet-digilent-zedboard-2020.2/pre-built/linux/images/rootfs.cpio.gz}
platform write
domain config -bif {}
platform write
platform generate
platform generate
platform generate
