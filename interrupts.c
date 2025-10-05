#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interrupts.h"

int main(int argc, char *argv[]){
    
    if (argc < 4) {
        fprintf(stderr, "Provide the necessary input files, usage: %s <trace_file> <vector_table_file> <device_table_file>\n", argv[0]);
        return 1;
    }

    const char *trace_file = argv[1];
    const char *vector_table_file = argv[2];
    const char *device_table_file = argv[3];

    char activities[MAX_ACTIVITIES][MAX_ACTIVITY_LENGTH];
    int values[MAX_ACTIVITIES]; // duration or device id per line
    int activityCount = 0;

    readTraceFile(trace_file, activities, values, &activityCount);

    int vectors[100];
    int vectorCount = 0;
    read_vector_table(vector_table_file, vectors, &vectorCount);
    if (vectorCount <= 0){
        fprintf(stderr, "No vectors were loaded.\n");
        return 1;
    }


    int device_times[300], has_device[300];
    read_device_table(device_table_file, device_times, has_device);

    
    FILE *execFile = fopen("execution.txt", "w");
    if(execFile == NULL){
        perror("Error creating execution.txt file");
        return 1;
    }


    int currentTime = 0;

    //Simulating each row given in trace.txt
    for (int i = 0; i < activityCount; i++) {
        if (strncmp(activities[i], "CPU", 3) == 0) {
            int d = values[i];
            recordOutput(execFile, currentTime, d, "CPU burst");
            currentTime += d;
        
        }else if(strncmp(activities[i], "SYSCALL", 7) == 0){
            int dev = values[i];
            if(dev < 0 || !has_device[dev]){
                fprintf(stderr, "Invalid device ID %d for SYSCALL\n", dev);
                continue;
            }
            handle_interrupt(execFile, &currentTime, dev, device_times[dev]);
        
        }else if(strncmp(activities[i], "END_IO", 6) == 0) {
            int dev = values[i];
            if(dev < 0 || !has_device[dev]){
                fprintf(stderr, "Unknown/invalid device id in END_IO: %d\n", dev);
                continue;
            }
            handle_interrupt(execFile, &currentTime, dev, device_times[dev]);

        }else{
            fprintf(stderr, "Unknown activity in trace.txt: \"%s\"\n", activities[i]);
        }
    }
}
