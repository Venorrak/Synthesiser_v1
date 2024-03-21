import { writable } from 'svelte/store';

export const setSoundEnabled = writable({
    isSoundEnabled: false
});