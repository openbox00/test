#include "myio.h"
#include "String.h"

extern char* pUSARTtxData;
extern SemaphoreHandle_t xSemUSART1send;

void _print(char *str){
    
	

    
    while( xSemaphoreTake(xSemUSART1send,  portMAX_DELAY) == pdFALSE);
    
    while(*str){
        if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET){
            USART_SendData(USART1, *str++);
        }
    }
    //The Interrup will always assert when the TXE bit (SR register) is empty(USART_IT_TXE == 1)
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
       
}

void qprintf(xQueueHandle tx_queue, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    int curr_ch = 0;
    char out_ch[2] = {'\0', '\0'};
    char newLine[3] = {'\n' , '\r', '\0'};
    char percentage[] = "%";
    char *str;
    char str_num[10];
    int out_int;

    /* Block for 1ms. */
     const portTickType xDelay = 0.1; // portTICK_RATE_MS;

    while( format[curr_ch] != '\0' ){
        vTaskDelay( xDelay ); 
        if(format[curr_ch] == '%'){
            if(format[curr_ch + 1] == 's'){
                str = va_arg(ap, char *);
                while (!xQueueSendToBack(tx_queue, str, portMAX_DELAY)); 
                //parameter(...,The address of a string which is put in the queue,...)
            }else if(format[curr_ch + 1] == 'd'){
                itoa(va_arg(ap, int), str_num);
                while (!xQueueSendToBack(tx_queue, str_num, portMAX_DELAY));                
            }else if(format[curr_ch + 1] == 'c'){
                out_ch[0] = (char)va_arg(ap, int);
                while (!xQueueSendToBack(tx_queue, out_ch, portMAX_DELAY));                                   
           }else if(format[curr_ch + 1] == 'x'){
                xtoa(va_arg(ap, int), str_num);
                while (!xQueueSendToBack(tx_queue, str_num, portMAX_DELAY));                                       
            }else if(format[curr_ch + 1] == '%'){
                while (!xQueueSendToBack(tx_queue, percentage, portMAX_DELAY));                                    
            }
            curr_ch++;
        }else if(format[curr_ch] == '\n'){
            while (!xQueueSendToBack(tx_queue, newLine, portMAX_DELAY));
        }else{
            out_ch[0] = format[curr_ch];
            while (!xQueueSendToBack(tx_queue, out_ch, portMAX_DELAY));         
        }
        curr_ch++;
    }//End of while
    va_end(ap);
}


//put string into default pipe defined in _print().
void uprintf(const char *format, ...){
    va_list ap;
    va_start(ap, format);
    int curr_ch = 0;
    char out_ch[2] = {'\0', '\0'};
    char newLine[3] = {'\n' , '\r', '\0'};
    char percentage[] = "%";
    char *str;
    char str_num[10];
    char str_out[30] = "";
    int nCnt = 0;

    while( format[curr_ch] != '\0' ){
        
        if(format[curr_ch] == '%'){

            switch(format[curr_ch + 1]){
                case 's':
                    str = va_arg(ap, char *);
                    //prevent str_out is no more space
                    nCnt = sizeof(str_out) - strlen(str_out);
                    strncat(str_out, str, nCnt - 1);
                    break;
                case 'd':
                    itoa(va_arg(ap, int), str_num);
                    nCnt = sizeof(str_out) - strlen(str_out);
                    strncat(str_out, str_num, nCnt - 1);
                    //str_out = str_num;
                    break;
                case 'c':                  
                    out_ch[0] = (char)va_arg(ap, int);
                    nCnt = sizeof(str_out) - strlen(str_out);
                    strncat(str_out, out_ch, nCnt - 1);
                    //str_out = out_ch;
                    break;
                case 'x':
                    xtoa(va_arg(ap, int), str_num);
                    nCnt = sizeof(str_out) - strlen(str_out);
                    strncat(str_out, str_num, nCnt - 1);
                    //str_out = str_num;
                    break;
                case '%':
                    nCnt = sizeof(str_out) - strlen(str_out);
                    strncat(str_out, percentage, nCnt - 1);
                    //str_out = percentage;
                    break;
                default:;
            }//End of switch(format[curr_ch + 1])

            curr_ch++;

        }else if(format[curr_ch] == '\n'){
            
            //str_out = newLine;
            //prevent str_out is no more space
            nCnt = sizeof(str_out) - strlen(str_out);
            strncat(str_out, newLine, nCnt - 1);
            
        }else{
            
            out_ch[0] = format[curr_ch];
            
            //prevent str_out is no more space
            nCnt = sizeof(str_out) - strlen(str_out);
            strncat(str_out, out_ch, nCnt);
        }
        curr_ch++;
    }//End of while( format[curr_ch] != '\0' )
    _print(str_out); //print on screen
    va_end(ap);
}//End of void printf(const char *format, ...)
