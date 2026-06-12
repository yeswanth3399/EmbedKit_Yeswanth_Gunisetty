//uart_parser.c

//requried header files
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//defines for UART frame
#define UART_SOF                0xAAU
#define UART_MAX_PAYLOAD        16U

//defines for the UART Frame validation
#define UART_FRAME_OK                       1
#define UART_FRAME_IN_PROGRESS              0
#define UART_CHECK_SUM_ERROR                -1
#define UART_TIME_OUT_ERROR                 -2

//defines for STATES MISSION
typedef enum
{
    UART_STATE_WAIT_SOF=0,
    UART_STATE_CMD,
    UART_STATE_LEN,
    UART_STATE_PAYLOAD,
    UART_STATE_CHECKSUM

}uart_state_t;

//Frame structure
typedef struct
{
    uart_state_t state;

    uint8_t cmd;
    uint8_t len;

    uint8_t payload[UART_MAX_PAYLOAD];
    uint8_t payload_index;
    uint8_t checksum;

    uint32_t last_timestamp_ms;
    uint32_t timeout_ms;
    uint32_t last_gap_ms;

}uart_parser_t;

//reset parser and preparing for new frame
static void uart_parser_reset(uart_parser_t *parser)
{
    parser->state = UART_STATE_WAIT_SOF;

    parser->cmd=0U;
    parser->len=0U;
    parser->payload_index=0U;

    parser->checksum=0U;
    memset(parser->payload,0,sizeof(parser->payload));
}

//Initialze the parser with configured timeout value
void uart_parser_init(uart_parser_t *parser, uint32_t timeout_ms)
{
    memset(parser,0,sizeof(*parser));

    parser->timeout_ms=timeout_ms;
    parser->state=UART_STATE_WAIT_SOF;
}

//Process a recived byte and upadte parser state
int uart_parser_feed_byte(uart_parser_t *parser, uint8_t byte, uint32_t timestamp_ms)
{
    uint32_t gap;

    //checks for timeout
    if((parser->state!=UART_STATE_WAIT_SOF)&&(parser->timeout_ms!=0U))
    {
        gap=timestamp_ms-parser->last_timestamp_ms;

        if(gap>parser->timeout_ms)
        {
            parser->last_gap_ms=gap;
            
            uart_parser_reset(parser);
            return UART_TIME_OUT_ERROR;
        }
    }

    switch(parser->state)
    {
        case UART_STATE_WAIT_SOF:
                if(byte==UART_SOF)
                {
                    parser->state= UART_STATE_CMD;
                }
                break;
        
        case UART_STATE_CMD:
                parser->cmd=byte;
                parser->checksum=byte;

                parser->state= UART_STATE_LEN;
                break;
        
        case UART_STATE_LEN:
                parser->len=byte;

                if(parser->len > UART_MAX_PAYLOAD)
                {
                    uart_parser_reset(parser);
                    break;
                }

                parser->checksum^=(byte);

                if(parser->len == 0U)
                {
                    parser->state= UART_STATE_CHECKSUM;
                }
                else
                {
                    parser->payload_index=0U;
                    parser->state= UART_STATE_PAYLOAD;
                }
                break;
        
        case UART_STATE_PAYLOAD:
                parser->payload[parser->payload_index++]=byte;

                parser->checksum^=(byte);

                if(parser->payload_index>=parser->len)
                {
                    parser->state= UART_STATE_CHECKSUM;
                }
                break;
        
        case UART_STATE_CHECKSUM:
                //printf("DEBUG: received=0x%02X calculated=0x%02X\n",byte,parser->checksum); //this printf is for debugging purpose
                if(byte == parser->checksum)
                {
                    parser->state= UART_STATE_WAIT_SOF;
                    parser->last_timestamp_ms=timestamp_ms;
                    return UART_FRAME_OK;
                }

                uart_parser_reset(parser);
                return UART_CHECK_SUM_ERROR;
        
        default: 
                uart_parser_reset(parser);
                break;
    }

    parser->last_timestamp_ms= timestamp_ms;
    return UART_FRAME_IN_PROGRESS;
}

//print UART Frame
static void print_frame(uart_parser_t *parser)
{
    uint8_t i;

    printf("CMD=0x%02X LEN=%u PAYLOAD=[",parser->cmd,parser->len);

    for(i=0U ;i<parser->len; i++)
    {
        printf("%02X",parser->payload[i]);

        if(i<parser->len -1U)
        {
            printf(" ");
        }
    }

    printf("]");
}

//feed a complete byte stream into the parser
static void feed_stream(uart_parser_t *parser,const uint8_t bytes[], const uint32_t times[], uint32_t count)
{
    uint32_t i;
    int result;

    for(i=0U; i<count; i++)
    {
        result = uart_parser_feed_byte(parser, bytes[i], times[i]);

        switch(result)
        {
            case UART_FRAME_IN_PROGRESS:
                        printf("t=%3ums byte=0x%02X -> receiving...\n",times[i],bytes[i]);
                        break;
            
            case UART_FRAME_OK:
                    printf("t=%3ums byte=0x%02X -> FRAME OK ",times[i],bytes[i]);

                    print_frame(parser);

                    printf("\n");

                    uart_parser_reset(parser);
                    break;
            
            case UART_CHECK_SUM_ERROR:
                    printf("t=%3ums byte=0x%02X -> CHECKSUM ERROR\n",times[i],bytes[i]);
                    break;
            
            case UART_TIME_OUT_ERROR:
                    printf("t=%3ums byte=0x%02X -> TIMEOUT (%ums gap > %ums) -- parser reset\n",times[i],bytes[i],parser->last_gap_ms,parser->timeout_ms);

                    /*Re-fed the same byte*/
                    (void)uart_parser_feed_byte(parser,bytes[i],times[i]);

                    printf("t=%3ums byte=0x%02X -> receiving... (re-fed after reset)\n",times[i],bytes[i]);
                    break;

            default:
                    break;
        }
    }
}

//test cases implementation
// Test Case 1:
// Valid frame received without timeout
static void test_case_1(void)
{
    uart_parser_t parser;

    uint8_t bytes[]=
    {
        0xAA,
        0x01,
        0x03,
        0x10,
        0x20,
        0x30,
        0x02
    };

    uint32_t times[]=
    {
        0,
        5,
        10,
        15,
        20,
        25,
        30
    };

    printf("\n================ TEST CASE 1 ================\n");

    uart_parser_init(&parser, 50U);

    feed_stream(&parser,bytes,times,(sizeof(bytes)/sizeof(bytes[0])));

}

// Test Case 2:
// Timeout occurs mid-frame and parser recovers
// using the next valid frame
static void test_case_2(void)
{
    uart_parser_t parser;

    uint8_t bytes[]=
    {
        0xAA,
        0x01,
        0x03,
        0x10,


        0xAA,
        0x05,
        0x01,
        0x7F,
        0x7B
    };

    uint32_t times[]=
    {
        0,
        5,
        10,
        15,

        200,
        205,
        210,
        215,
        220
    };

    printf("\n================ TEST CASE 2 ================\n");
    uart_parser_init(&parser,50U);

    feed_stream(&parser, bytes,times,(sizeof(bytes)/sizeof(bytes[0])));
}

// Test Case 3:
// Two valid frames received back-to-back
static void test_case_3(void)
{
    uart_parser_t parser;

    uint8_t bytes[]=
    {
       0xAA,
       0x03,
       0x01,
       0x55,
       0x57,

       0xAA,
       0x04,
       0x02,
       0xAA,
       0xBB,
       0x17
    };

    uint32_t times[]=
    {
        0,5,10,15,20,
        25,30,35,40,45,50
    };

    printf("\n================ TEST CASE 3 ================\n");
    uart_parser_init(&parser,50U);

    feed_stream(&parser, bytes, times, (sizeof(bytes)/sizeof(bytes[0])));
}

// Test Case 4:
// Same data as Test Case 2 with timeout disabled
static void test_case_4(void)
{
    uart_parser_t parser;

    uint8_t bytes[]=
    {
        0xAA,
        0x01,
        0x03,
        0x10,

        0xAA,
        0x05,
        0x01,
        0x7F,
        0x7B
    };

    uint32_t times[]=
    {
        0,
        5,
        10,
        15,

        200,
        205,
        210,
        215,
        220
    };

    printf("\n================ TEST CASE 4 ================\n");
    printf("Timeout Disabled (timeout = 0)\n");

    uart_parser_init(&parser,0U);

    feed_stream(&parser,bytes,times,(sizeof(bytes)/sizeof(bytes[0])));
}

int main(void)
{
    test_case_1();

    test_case_2();

    test_case_3();

    test_case_4();
    return 0;
}