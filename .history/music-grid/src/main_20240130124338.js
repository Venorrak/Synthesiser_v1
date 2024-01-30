import App from './App.svelte';

var app = new App({
	target: document.body
});

let noteElement = document.body.getElementsByClassName('svelte-noro3b playing');
if (noteElement.length > 0) { // Check if noteElement exists
	console.log('scrolling');
	noteElement[0].scrollIntoView({ behavior: 'smooth', block: 'center' }); // Use noteElement[0] to access the first element
}
export default app;