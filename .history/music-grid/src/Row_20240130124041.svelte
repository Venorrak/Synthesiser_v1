<script>
	import Cell from './Cell.svelte'
	import { playRow } from './Music.svelte'
	import { scrollIntoView } from 'svelte/internal' // Add this import

	export let row = [];
	export let playing = false;
	export let paused = false;

	const playThisRow = () => {
		row.isPlaying = true;
		playRow(row);
		setTimeout(() => {
			row.isPlaying = false;
			let noteElement = document.getElementsByClassName('svelte-noro3b playing');
			console.log(noteElement);
			if (noteElement.length > 0) { // Check if noteElement exists
				noteElement[0].scrollIntoView({ behavior: 'smooth', block: 'center' }); // Use noteElement[0] to access the first element
			}
		}, 500);
	}
</script>

<style>
	tr.playing {
		transform: scale(0.95);
	}
	tr {
		transition: 0.1s ease-in all;
	}

	i.row-play {
		margin-top: 7px;
		margin-right: 4px;
		font-size: 14px;
		color: dimgrey;
		cursor: pointer;
	}
	i.row-play:hover {
		color: white;
	}
</style>

<tr class:playing={playing}>
	<i on:click={playThisRow} class="row-play fa fa-sm fa-inverse fa-play"/>
	{#each row as cell, index}
		<Cell bind:active={cell} column="{index}" paused={paused}></Cell>
	{/each}
</tr>