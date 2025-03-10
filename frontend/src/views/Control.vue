<template>
    <v-container class="container" fluid justify="center" align="center">
        <v-row style="max-width: 1200px;" justify="center" class="d-flex flex-wrap ga-12">
            <!-- COLUMN 1 -->
            <v-col align="center" class="d-flex flex-wrap">
                <!-- SHEET 1 -->
                <v-sheet class="rounded-t-lg mb-1" color="surface" elevation="0" style="max-width: 800px; width: 100%;">
                    <v-card class="text-secondary" title="LED CONTROLS" color="surface" subtitle="Recent settings" flat variant="tonal" ></v-card>
                </v-sheet>
                <!-- SHEET 2 -->
                <v-sheet class="mb-1" color="surface" elevation="0" style="max-width: 800px; width: 100%;">
                    <v-card class="pt-5" color="surface" variant="tonal">
                        <v-slider class="pt-2 bg-surface" append-icon="mdi:mdi-car-light-high" v-model="led.brightness" density="compact" thumb-size="16" color="secondary" label="Brightness" direction="horizontal" min="0" max="250" step="10" show-ticks thumb-label="always"></v-slider>
                    </v-card>
                </v-sheet>
                <!-- SHEET 3 -->
                <v-sheet class="mb-1" justify="center" align="center" color="surface" elevation="0" style="max-width: 800px; width: 100%;">
                    <v-card class="pt-5" color="surface" variant="tonal">
                        <v-slider class="pt-2 bg-surface" append-icon="mdi:mdi-led-on" v-model="led.nodes" density="compact" thumb-size="16" color="secondary" label="LED Nodes" direction="horizontal" min="1" max="7" step="1" show-ticks thumb-label="always"></v-slider>
                    </v-card>
                </v-sheet>
                <!-- SHEET 4 -->
                <v-sheet class="mb-1 pa-2 border" justify="center" align="center" color="surface" elevation="0" style="max-width: 800px; width: 100%;">
                    <v-progress-circular rotate="0" size="200" width="15" :model-value="led.nodes *15" :color="indicatorColor">
                        <span class="text-onSurface font-weight-bold">{{ led.nodes }} LED(s)</span>
                    </v-progress-circular>
                </v-sheet>
            </v-col>
            <!-- COLUMN 2 -->
            <v_col align="center">
                <v-color-picker v-model="led.color"></v-color-picker>
            </v_col>
        </v-row>
    </v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { useMqttStore } from "@/store/mqttStore"; // Import Mqtt Store
import { storeToRefs } from "pinia";
import { computed, onBeforeUnmount, onMounted, reactive, watch, ref } from "vue";
import { useRoute, useRouter } from "vue-router";
 
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  

const Mqtt        = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);

const led         = reactive({"brightness":255, "nodes":1, "color":{r: 255, g: 0, b: 255, a: 1}})
let timer, ID     = 1000;

// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    Mqtt.connect(); // Connect to Broker located on the backend

    setTimeout( ()=>{
        //Subscribe to each topic
        Mqtt.subscribe("620154033");
        Mqtt.subscribe("620154033_pub");
    }, 3000);
});

onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    // unsubcribe from all topics
    Mqtt.unsubcribeAll();
});

// WATCHERS
watch(led,(controls)=>{
    clearTimeout(ID);
    ID = setTimeout(()=>{
        const message =
        JSON.stringify({"type":"controls","brightness":controls.brightness,"leds":controls.nodes,"color": controls.color});
        Mqtt.publish("620154033_sub",message); // Publish to a topic subscribed to by the hardware
    },1000);
})

// COMPUTED PROPERTIES
const indicatorColor = computed(()=>{
    return `rgba(${led.color.r},${led.color.g},${led.color.b},${led.color.a})`
})

</script>


<style scoped>
/** CSS STYLE HERE */


</style>
  