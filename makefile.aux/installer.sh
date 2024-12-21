#!/bin/bash

main(){
    BinaryName="$3"
    if [ "$1" != "__fomrMakeFile__"]; then
        echo "This script is not meant to be executed directly. Instead, use 'make install' or 'make uninstall' to install or uninstall the project."
    fi

    #check is running as root
    if [ "$EUID" -ne 0 ]; then
        echo "Please run as root"
        exit
    fi

    if [ "$2" == "install" ]; then
        install
    elif [ "$2" == "uninstall" ]; then
        uninstall
    elif [ "$2" == "purge" ]; then
        purge
    else
        echo "Invalid argument. Use 'install' or 'uninstall' to install or uninstall the project."
    fi

}

install(){
    cp -r build/.sysroot/* /
    cp "build/$BinaryName" /usr/bin/
}

uninstall(){
    rm -rf /usr/bin/"$BinaryName"
    rm -rf /etc/vss
    rm -rf /etc/ssl/certs/vssCert.pem
    rm -rf /etc/ssl/certs/vssKey.pem
}

purge(){
    uninstall
    rm -rf /var/vss
    rm /var/log/vss.log
}

main "$@"