<template>
    <v-container class="bg-surface" fluid></v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { onBeforeUnmount, onMounted } from "vue";
import { useRoute, useRouter } from "vue-router";
 
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  


// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
});


</script>


<style scoped>
/** CSS STYLE HERE */


</style>
  