#ifndef SENSOR_CONTROL_H_
#define SENSOR_CONTROL_H_

#define IMX576_PWREN_PIN    (17)
#define IMX576_RESET_PIN    (10)
#define IMX586_RESET_PIN    (20)
#define IMX586_PWREN_PIN    (19)
#define SENSOR_MIPI_MUX_PIN (11)
#define TEST_JUMPER_WIRES_PIN (9)

void sensor_control_io_init(void);
void imx576_power_enable(void);
void imx576_power_disable(void);
void imx586_power_enable(void);
void imx586_power_disable(void);
void sensor_both_power_enable(void);
void sensor_control_mipi_mux_to_imx214(void);
void sensor_control_mipi_mux_to_imx586(void);

#endif
