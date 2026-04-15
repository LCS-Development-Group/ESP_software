#pragma once
#include "common_includes.h"
#include "I2C.h"

class t_sensor
{
protected:
    uint8_t port;
    uint8_t addr;
    SemaphoreHandle_t mutex;
    bool is_initialized;
    bool proceed_when_fail;


public:
    t_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail);

    uint8_t get_port() const {return port;}
    uint8_t get_addr() const {return addr;}
    bool get_is_initialized() const {return is_initialized;}
    bool get_proceed_when_fail() const {return proceed_when_fail;}
    SemaphoreHandle_t *get_mutex_ptr() {return &mutex;}
    SemaphoreHandle_t get_mutex() {return mutex;}

    virtual void reset_variable()=0;
    virtual void reset_variable_without_mutex()=0;
    virtual void connection_initialize()=0;
    virtual void take_readings()=0;
};

class t_RHT_sensor: public t_sensor
{
    float default_RH;
    float default_T;

    float RH;
    float T;

    sht3x_t* dev;

public:
    t_RHT_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail, float _default_RH, float _default_T);
    ~t_RHT_sensor();

    float get_RH() const {return RH;}
    float get_T() const {return T;}

    float* get_RH_ptr() {return &RH;}
    float* get_T_ptr() {return &T;}

    void reset_variable() override;
    void reset_variable_without_mutex() override;
    void connection_initialize() override;
    void take_readings() override;
};

class t_INA_sensor: public t_sensor
{
    float default_current;
    float default_voltage;
    float default_power;

    float current;
    float voltage;
    float power;

    ina219_t *dev;

public:
    t_INA_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail, float _default_current, float _default_voltage, float _default_power);
    ~t_INA_sensor(){if(dev!=nullptr) delete dev;}

    float get_current() const {return current;}
    float get_voltage() const {return voltage;}
    float get_power() const {return power;}

    float* get_current_ptr() {return &current;}
    float* get_voltage_ptr() {return &voltage;}
    float* get_power_ptr() {return &power;}

    void reset_variable() override;
    void reset_variable_without_mutex() override;
    void connection_initialize() override;
    void take_readings() override;
};


/*
    Boilerplate INA
Main (poprawne wywołanie)
    Popraw COMM
    Popraw REG
    Popraw GUI

TESTY

cała INA

TESTY

reintegracja informacji o braku sensora przy włączaniu i informacja o rozłączeniu

*/