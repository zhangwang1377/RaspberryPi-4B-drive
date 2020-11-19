## 设备树说明

* 官方说明文档[链接](https://www.raspberrypi.org/documentation/configuration/device-tree.md)

***

*  安装设备树编译器
~~~ sh
sudo apt install device-tree-compiler
~~~

* 编译命令
~~~sh
dtc -I dts -O dtb -o pi-led-overlay.dtbo pi-led-overlay.dts
~~~

* 将 **pi-led-overlay.dtbo** 复制到 **/boot/overlays** 
~~~ sh
sudo cp i-led-overlay.dtbo /boot/overlays
~~~
* 在 **/boot/config.txt** 最后一行添加 **dtoverlay=pi-led-overlay**

* 重启 **reboot**

* 可以在 **/proc/device-tree** 或者 **/sys/firmware/devicetree/base** 下查看设备树信息是否注册成功
~~~sh
pi@wang:~ $ ls /proc/device-tree
'#address-cells'   emmc2bus             my_pi_led0        serial-number
 aliases           fixedregulator_3v3   my_pi_led1       '#size-cells'
 arm-pmu           fixedregulator_5v0   name              soc
 axi               gpu                  __overrides__     __symbols__
 chosen            interrupt-parent     phy               system
 clk-108M          leds                 reserved-memory   thermal-zones
 clocks            memory@0             scb               timer
 compatible        memreserve           sd_io_1v8_reg     v3dbus
 cpus              model                sd_vcc_reg
~~~
***

* **dts demo**
~~~dts
/dts-v1/;
/plugin/;

/ {
	compatible = "brcm,bcm2835";

	fragment@0 {
		target-path = "/";
		__overlay__ {
            my_pi_led0 {
                compatible = "pi_4b,my_pi_led";
                pin = <20>;

            };

			my_pi_led1 {
                compatible = "pi_4b,my_pi_led";
                pin = <21>;

            };
		};
	};
};

~~~


