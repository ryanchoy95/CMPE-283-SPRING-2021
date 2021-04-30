# Assignment 2
# Steps:
1. Downloaded a stable version of linux.
2. In the linux file, used the cp /boot/config/linux-5.11.14 ./.config command, followed by make old config.
3. Ran make modules && make && sudo make install && sudo make install_modules for initial setup and after code changes.
4. Configured the cpuid.c code in the kvm_emulate_cpuid function and vmx_handle_exit function in vmx.c
5. Installed virt-manager and ran a vm with cpuid installed
6. rand the cpuid commannd cpuid -l 0x4FFFFFFF in inner vm
