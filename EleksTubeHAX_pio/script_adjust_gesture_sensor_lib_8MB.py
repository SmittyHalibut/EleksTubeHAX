def inplace_change(filename, old_string, new_string):
    # Safely read the input filename using 'with'
    with open(filename) as f:
        s = f.read()
        if old_string not in s:
            print('"{old_string}" not found in {filename}.'.format(**locals()))
            return
        else:
            print('Found "{old_string}" in {filename}. Trying to change now to the new value!'.format(**locals()))

    # Safely write the changed content, if found in the file
    with open(filename, 'w') as f:
        #print('Found Changing "{old_string}" to "{new_string}" in {filename}'.format(**locals()))
        s = s.replace(old_string, new_string)
        f.write(s)

print("===== adjusting APDS sensor libs ===== ")

new_string_to_check = "#define APDS9960_ID_3"

filename1 = "./.pio/libdeps/esp32dev8MB/SparkFun APDS9960 RGB and Gesture Sensor/src/SparkFun_APDS9960.h"
filename2 = "./.pio/libdeps/esp32dev8MB/SparkFun APDS9960 RGB and Gesture Sensor/src/SparkFun_APDS9960.cpp"
with open(filename1) as f:
    s = f.read()
    if new_string_to_check not in s:
        #do replacements
        inplace_change(filename1,"#define APDS9960_ID_2           0x9C ","#define APDS9960_ID_2           0x9C\r#define APDS9960_ID_3           0xA8")
        inplace_change(filename2,"if( !(id == APDS9960_ID_1 || id == APDS9960_ID_2) ) {","if( !(id == APDS9960_ID_1 || id == APDS9960_ID_2 || id == APDS9960_ID_3) ) {")
    else:
        print("Lib files seems to be adjusted already! Doing nothing.")
print("Done adjusting APDS sensor libs!")