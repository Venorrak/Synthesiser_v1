import App from './App.svelte';

var app = new App({
	target: document.body
});

let noteElement = document.getElementsByClassName('svelte-noro3b playing');
console.log(noteElement);
if (noteElement.length > 0) { // Check if noteElement exists
	noteElement[0].scrollIntoView({ behavior: 'smooth', block: 'center' }); // Use noteElement[0] to access the first element
}
export default app;