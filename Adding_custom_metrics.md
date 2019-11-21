

## Dynamic metrics

To add a customized metric for measuring execution time cycles between two 
points. First extend <IoT2CntrStruct>and  <IoT2GlobalCounter> in IoT2Lib.h and
IoT2Lib.c respectively.


```C

/* BenchIoT/IoT2/IoT2-lib/IoT2Lib.h */

typedef volatile struct
{
    // Total runtime
    IoT2RuntimeCntr RuntimeCycles;
    // Core metrics
    IoT2MetricCntr ISRCycles;
    IoT2MetricCntr SVCCycles;
    IoT2MetricCntr PendSVCycles;
    IoT2MetricCntr SysTickCycles;
    IoT2MetricCntr IoT2Cycles;          
    // Security
    IoT2MetricCntr PrivCycles;
    IoT2MetricCntr PrivThreadCycles;
    IoT2MetricCntr UnprivCycles;
    // Performance 
    IoT2MetricCntr SleepCycles;
    IoT2MetricCntr DebugCycles;        // Additional counter for debugging
    // Add your customized metric below
    IoT2MetricCntr CustomMetric;

} IoT2CntrStruct;

};
```


```C
/* BenchIoT/IoT2/IoT2-lib/IoT2Lib.c */
IoT2CntrStruct IoT2GlobalCounter = {
    // Total runtime
    .RuntimeCycles = {0},
    // Core metrics
    .ISRCycles = {0},
    .SVCCycles = {0},
    .PendSVCycles = {0},
    .SysTickCycles = {0},
    .InitCycles = {0},
    .IoT2Cycles = {0},
    // Security metrics
    .PrivCycles = {0},
    .PrivThreadCycles = {0},
    .UnprivCycles = {0},
    // Performance metrics
    .SleepCycles = {0},
    // extra counter for debugging
    .DebugCycles = {0},
    // Add the initialization to your custom metric below
    .CustomMetric = {0}
};
```

Then, you can use the following API to start the measurement

```C
void iot2StartCustomeMetric(IoT2MetricCntr *metric)
```

To end the measurement, 

```C
void iot2EndCustomeMetric(IoT2MetricCntr *metric)
```

Now this will allow collecting the metric. The last step is to extend the 
result collector wrapper at (iot2ResultCollector.cpp). This allows sending 
the result through the serial port.

***Note that you must add the following before the end_collector_msg***

```C
/* BenchIoT/IoT2/IoT2-mbed-wrappers/result_collection/iot2ResultCollector.cpp */
    //--------------------------------------------------------------------------
    // Your customized metric
    //--------------------------------------------------------------------------

    memset(result_buff, 0, sizeof(result_buff));

    if (snprintf(result_buff, sizeof(result_buff), 
        "CustomMetric cycles:%" PRIu32 "", 
        IoT2GlobalCounter.CustomMetric.metricCntr) < 0){

        // an error occured!
        result_collector.printf(
            "[-] ERROR: could not perform snprintf on CustomMetric cycles\r\n");
    }

    result_collector.printf("IoT2->%s\r\n", result_buff);


    //--------------------------------------------------------------------------
    //                   send end message to notify the result collector
    //--------------------------------------------------------------------------

```

For other dynamic metrics, this can be done by extending IoTLib files.


## Static metrics

For static metrics, these can be added through python scripts at 
```
BenchIoT/IoT2/IoT2_bench_drivers
```

Check iot2_measure_static_flash_and_ram.py for an example. You can also automate
collecting your static metric at the end of iot2_run_experiments.py or 
iot2_run_benchmark.py.