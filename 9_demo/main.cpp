#include "mbed.h"
#include "mbed_rpc.h"
#include "mbed_events.h"

/**
 *  This example program has been updated to use the RPC implementation in the new mbed libraries.
 *  This example demonstrates using RPC over serial
**/
RpcDigitalOut myled1(LED1,"myled1");
RpcDigitalOut myled2(LED2,"myled2");
RpcDigitalOut myled3(LED3,"myled3");
BufferedSerial pc(USBTX, USBRX);

void LEDblink(Arguments *in, Reply *out);
RPCFunction rpcLED(&LEDblink, "LEDblink");
void LEDblink_exe(void);

void LEDblink_stop(Arguments *in, Reply *out);
RPCFunction rpcLED_stop(&LEDblink_stop, "LEDblink_stop");

char done = 0;
EventQueue eventQueue;

int main() {
    //The mbed RPC classes are now wrapped to create an RPC enabled version - see RpcClasses.h so don't add to base class
	Thread eventThread(osPriorityNormal);
	eventThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));
    // receive commands, and send back the responses
    char buf[256], outbuf[256];

    FILE *devin = fdopen(&pc, "r");
    FILE *devout = fdopen(&pc, "w");

    while(1) {
        memset(buf, 0, 256);
        for (int i = 0; ; i++) {
            char recv = fgetc(devin);
            if (recv == '\n') {
                printf("\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        //Call the static call method on the RPC class
        RPC::call(buf, outbuf);
        printf("%s\r\n", outbuf);
    }
}

// Make sure the method takes in Arguments and Reply objects.
void LEDblink (Arguments *in, Reply *out)   {
	eventQueue.call(LEDblink_exe);
}

void LEDblink_exe(void)
{
	int which_on = 1;

	done = 0; // means not done yet
    // Have code here to call another RPC function to wake up specific led or close it.
    char buffer[200], outbuf[256];
    char strings[20];

	while (!done) {
		if (which_on == 1) {
			which_on = 0;
			memset(buffer, 0, 200);
			sprintf(strings, "/myled2/write 1");
			strcpy(buffer, strings);
			RPC::call(buffer, outbuf);

			memset(buffer, 0, 200);
			sprintf(strings, "/myled3/write 0");
			strcpy(buffer, strings);
			RPC::call(buffer, outbuf);
		}
		else if (which_on == 0) {
			which_on = 1;
			memset(buffer, 0, 200);
			sprintf(strings, "/myled2/write 0");
			strcpy(buffer, strings);
			RPC::call(buffer, outbuf);

			memset(buffer, 0, 200);
			sprintf(strings, "/myled3/write 1");
			strcpy(buffer, strings);
			RPC::call(buffer, outbuf);
		}

		ThisThread::sleep_for(400ms);
	}
}
void LEDblink_stop(Arguments *in, Reply *out)
{
	done = 1;
}
