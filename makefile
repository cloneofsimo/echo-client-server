.PHONY : echo-client echo-server uc us clean install uninstall android-install android-uninstall

all: echo-client echo-server uc us

echo-client:
	cd echo-client; make; cd ..

echo-server:
	cd echo-server; make; cd ..

uc:
	cd uc; make; cd ..

us:
	cd us; make; cd ..

clean:
	cd echo-client; make clean; cd ..
	cd echo-server; make clean; cd ..
	cd uc; make clean; cd ..
	cd us; make clean; cd ..

install:
	sudo cp bin/echo-client /usr/sbin
	sudo cp bin/echo-server /usr/sbin
	sudo cp bin/uc /usr/sbin
	sudo cp bin/us /usr/sbin

uninstall:
	sudo rm /usr/sbin/echo-client /usr/sbin/echo-server /usr/sbin/uc /usr/sbin/us

android-install:
	adb push bin/echo-client bin/echo-server bin/uc bin/us /data/local/tmp
	adb exec-out "su -c mount -o rw,remount /system"
	adb exec-out "su -c cp /data/local/tmp/echo-client /data/local/tmp/echo-server /data/local/tmp/uc /data/local/tmp/us /system/xbin"
	adb exec-out "su -c chmod 755 system/xbin/echo-client"
	adb exec-out "su -c chmod 755 system/xbin/echo-server"
	adb exec-out "su -c chmod 755 system/xbin/uc"
	adb exec-out "su -c chmod 755 system/xbin/us"
	adb exec-out "su -c mount -o ro,remount /system"
	adb exec-out "su -c rm /data/local/tmp/echo-client /data/local/tmp/echo-server /data/local/tmp/uc /data/local/tmp/us"

android-uninstall:
	su -c mount -o rw,remount /system
	su -c rm /system/xbin/echo-client /system/xbin/echo-server /system/xbin/uc /system/xbin/us
	su -c mount -o ro,remount /system
