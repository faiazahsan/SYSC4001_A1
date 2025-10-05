#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdio.h>

#define MAX_ACTIVITIES 1000
#define MAX_ACTIVITY_LENGTH 50
#define NUMBER_OF_DEVICES 256


/*
    this function writes a single in the output text file execution.txt
*/
void recordOutput(FILE *executionFile, int currTime, int duration, const char *eventType){
    fprintf(executionFile, "%d, %d, %s\n", currTime, duration, eventType);
}



/*
    Reads the trace file which is trace.txt. Each line in the text file has two comma separated values.
    The first value is stored in the activities array and the second value is stored in the values array.
*/

void readTraceFile(const char *filename, char activities[][MAX_ACTIVITY_LENGTH], int values[], int *activityCount){
    FILE *file = fopen(filename, "r");

    if(file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[250];
    *activityCount = 0;

    while(fgets(line, sizeof(line), file)){
        
        char *p = line;
        
        //skip leading whitespace if it exists
        while(isspace(*p)){
            p++;
        } 
        
        if(*p == '\0') continue;

        //Reead one line, it should have the activity(SYSCALL or CPU) and the value(duration or device id)
        if(sscanf(p, "%29[^,], %d", activities[*activityCount], &values[*activityCount]) == 2){
            
            //remove trailing whitespace from activity name
            size_t len = strlen (activities[*activityCount]);
            if(len == 0) return;

            while (len > 0 && isspace(activities[*activityCount][len - 1])) {
                activities[*activityCount][len - 1] = '\0';
                len--;
            }

            (*activityCount)++;

        }
    }
    fclose(file);

}


/*
Reads the vector table from the file vector_table.txt. Each line has a hexadecimal ISR address.
The ISR addresses are stored the vectors array.
*/

void read_vector_table(const char *filename, int vectors[], int *vector_count){
    FILE *file = fopen(filename, "r");
    
    if(file == NULL){
        perror("Error opening vector_table.txt file");
        exit(EXIT_FAILURE);
    }

    char line[64];
    *vector_count = 0;

    while (fgets(line, sizeof line, file)){
        int value;
        if (sscanf(line, "%x", &value) == 1) { //reads hexadecimal values.
            vectors[*vector_count] = value;
            (*vector_count)++;
        }
    }
    fclose(file);
}


/*
This function reads the device table from the file device_table.txt where each line 
contains the duration for each device. has_entry array is used to mark which device ids have an entry in the file.
*/


void read_device_table(const char *filename, int device_times[], int *has_entry){
    
    for (int i = 0; i < NUMBER_OF_DEVICES; ++i) { 
        device_times[i] = -1; 
        has_entry[i] = 0;
    }

    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        perror("Error opening device_table.txt file");
        exit(EXIT_FAILURE);
    }

    char line[128];
    int next_idx = 0;
    while (fgets(line, sizeof line, f)){
        int dev, delay;
        if(sscanf(line, " %d", &delay) == 1){
            device_times[next_idx] = delay; 
            has_entry[next_idx] = 1; // works more like a validity check that the device entry exists
            next_idx++;
        }
    }
    fclose(f);
}



/*
Simulates the handling of an interrupt when a CPU is busy with some other execution.
*/
void handle_interrupt(FILE *exec_file, int *currTime, int intr_num, int device_total_ms){
    const int t_switch = 1, t_save = 10, t_find = 1, t_get = 1, t_iret = 1;
    const int overhead = t_switch + t_save + t_find + t_get + t_iret;
    int isr_body = 40;

    //switch to kernel mode
    recordOutput(exec_file, *currTime, t_switch, "switch to kernel mode");
    *currTime += t_switch;

    
    recordOutput(exec_file, *currTime, t_save, "context saved");
    *currTime += t_save;

    //find vector in the given memory position(each vector is 2 bytes)
    int mem_pos = intr_num * 2;
    char buf[100];
    snprintf(buf, sizeof buf, "find vector %d in memory position %d", intr_num, mem_pos);
    recordOutput(exec_file, *currTime, t_find, buf);
    *currTime += t_find;

    //Get the ISR address from the vrector table
    recordOutput(exec_file, *currTime, t_get, "obtain ISR address");
    *currTime += t_get;

    //the ISR body will be executed, it will call the appropriate device driver
    recordOutput(exec_file, *currTime, isr_body, "call device driver");
    *currTime += isr_body;

    //IRET
    recordOutput(exec_file, *currTime, t_iret, "IRET");
    *currTime += t_iret;

}

#endif