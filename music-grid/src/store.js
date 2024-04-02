import { writable } from 'svelte/store';

// Conserve the state of sound enabled/disabled
export const setSoundEnabled = writable({
    isSoundEnabled: false
});