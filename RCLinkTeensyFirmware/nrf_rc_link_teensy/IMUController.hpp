
//how often to send data to the computer
#define IMU_COMM_UPDATE_DELAY 100


namespace imu
{

    

    void imu_setup();
    void calibrate();

    //void get_acceleration_xyz(float *x, float *y, float *z);
    void update_rotation_ypr();

    void time_conditional_send_ypr();

    void imu_calibrate();

    extern unsigned long last_imu_comm_update_millis;



}