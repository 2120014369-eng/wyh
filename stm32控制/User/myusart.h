#ifndef __MYUSART_H
#define __MYUSART_H

void usart_init(void);
void my_send(char ch);
void my_send_string(const char *str, int num);
int my_usart_read_byte(char *ch);
void my_usart_irq_handler(void);
uint32_t my_usart_rx_overflow_count(void);

#endif
