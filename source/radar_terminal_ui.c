/*****************************************************************************
** File name: radar_terminal_ui.c
**
** Description: This file implements a terminal UI to configure parameters
** for presence detection application.
**
** ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG. All rights reserved.
** ===========================================================================
**
** ===========================================================================
** Infineon Technologies AG (INFINEON) is supplying this file for use
** exclusively with Infineon's sensor products. This file can be freely
** distributed within development tools and software supporting such
** products.
**
** THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
** OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
** INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON
** WHATSOEVER.
** ===========================================================================
*/

/* Header file from system */
#include <ctype.h>
#include <stdlib.h>

/* Header file includes */
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyhal.h"

/* Header file for local task */
#include "radar_terminal_ui.h"

/*******************************************************************************
 * Constants
 *******************************************************************************/
#define IFX_RADAR_SENSING_VALUE_MAXLENGTH 256

/*******************************************************************************
 * Function Name: terminal_ui_menu
 ********************************************************************************
 * Summary:
 *   This function prints the available parameters configurable for presence
 *   detection application. The existing values of the parameters are also
 *   displayed with their units (if any).
 *
 * Parameters:
 *   none
 *
 * Return:
 *   none
 *******************************************************************************/
static void terminal_ui_menu(void)
{
    char value[IFX_RADAR_SENSING_VALUE_MAXLENGTH];
    radar_presence_task_set_mute(true);

    /* Print main menu */
    printf("Select a setting to configure\n");
    mtb_radar_sensing_get_parameter(&sensing_context,
                                  "radar_presence_range_max",
                                  value,
                                  IFX_RADAR_SENSING_VALUE_MAXLENGTH);
    printf("'r': Set presence max range (%s)\n", value);
    mtb_radar_sensing_get_parameter(&sensing_context,
                                  "radar_presence_sensitivity",
                                  value,
                                  IFX_RADAR_SENSING_VALUE_MAXLENGTH);
    printf("'s': Set sensitivity (%s)\n", value);
    printf("\n");

    radar_presence_task_set_mute(false);
}

/*******************************************************************************
 * Function Name: terminal_ui_info
 ********************************************************************************
 * Summary:
 *   This function prints character using which a user can see all available
 *   parameters configurable for presence detection.
 *
 * Parameters:
 *   none
 *
 * Return:
 *   none
 *******************************************************************************/
static void terminal_ui_info(void)
{
    printf("Press '?' to list all radar presence settings\n");
}

/*******************************************************************************
 * Function Name: terminal_ui_readline
 ********************************************************************************
 * Summary:
 *   This function reads values entered by a user and prints it.
 *
 * Parameters:
 *   uart_ptr: UART object
 *   line: value to be read
 *   maxlength: maximum number of characters to be read
 *
 * Return:
 *   none
 *******************************************************************************/
static void terminal_ui_readline(void *uart_ptr, char *line, int maxlength)
{
    radar_presence_task_set_mute(true);

    if (maxlength <= 0)
    {
        return;
    }

    int i = 0;
    uint8_t rx_value = 0;
    /* Receive character until enter has been pressed */
    while ((rx_value != '\r') && (--maxlength > 0))
    {
        cyhal_uart_getc(uart_ptr, &rx_value, 0);
        cyhal_uart_putc(uart_ptr, rx_value);
        if (isspace(rx_value))
        {
            continue;
        }
        line[i++] = rx_value;
    }
    cyhal_uart_putc(uart_ptr, '\n');
    line[i] = '\0';

    radar_presence_task_set_mute(false);
}

/*******************************************************************************
 * Function Name: terminal_ui_print_result
 ********************************************************************************
 * Summary:
 *   This function displays a success/error message once a parameter has been
 *   configured.
 *
 * Parameters:
 *   result: success/error status message
 *
 * Return:
 *   none
 *******************************************************************************/
static void terminal_ui_print_result(cy_rslt_t result)
{
    switch (result)
    {
        case MTB_RADAR_SENSING_SUCCESS:
            printf("OK\n");
            break;
        default:
            printf("ERROR\n");
    }
}

/*******************************************************************************
 * Function Name: radar_presence_terminal_ui
 ********************************************************************************
 * Summary:
 *   Continuously checks if a key has been pressed to configure presence
 *   detection parameters. Displays a status message according to the user
 *   input/selection.
 *
 * Parameters:
 *   arg: thread
 *
 * Return:
 *   none
 *******************************************************************************/
void radar_presence_terminal_ui(cy_thread_arg_t arg)
{
    terminal_ui_menu();
    char value[IFX_RADAR_SENSING_VALUE_MAXLENGTH];
    uint8_t rx_value;

    /* Check if a key was pressed */
    while (cyhal_uart_getc(&cy_retarget_io_uart_obj, &rx_value, 0) == CY_RSLT_SUCCESS)
    {
        switch ((char)rx_value)
        {
            // menu
            case '?':
                terminal_ui_menu();
                break;
            // presence range max
            case 'r':
                printf("Enter range [0.66-10.2]m, press enter\n");
                terminal_ui_readline(&cy_retarget_io_uart_obj, value, IFX_RADAR_SENSING_VALUE_MAXLENGTH);
                terminal_ui_print_result(
                    mtb_radar_sensing_set_parameter(&sensing_context, "radar_presence_range_max", value));
                break;
            // sensitivity
            case 's':
                printf("Set Sensitivity: 'high', 'medium' or 'low'\n");
                terminal_ui_readline(&cy_retarget_io_uart_obj, value, IFX_RADAR_SENSING_VALUE_MAXLENGTH);
                terminal_ui_print_result(
                    mtb_radar_sensing_set_parameter(&sensing_context, "radar_presence_sensitivity", value));
                break;
            default:
                terminal_ui_info();
        }
        rx_value = 0;
    }
    printf("Exiting terminal ui\n");
    /* Exit current thread (suspend) */
    (void)cy_rtos_exit_thread();
}
