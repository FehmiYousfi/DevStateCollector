for service in $(busctl list | awk '{print $1}'); do
    echo "Service: $service" >> dbus_services_info.txt
    busctl introspect $service / >> dbus_services_info.txt
    echo -e "\n\n" >> dbus_services_info.txt
done

