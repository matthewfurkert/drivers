cmd_/home/furky/workspace/ldd/custom_drivers/004_pcd_platform_driver/Module.symvers := sed 's/ko$$/o/' /home/furky/workspace/ldd/custom_drivers/004_pcd_platform_driver/modules.order | scripts/mod/modpost -m    -o /home/furky/workspace/ldd/custom_drivers/004_pcd_platform_driver/Module.symvers -e -i Module.symvers   -T -
