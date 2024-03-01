<script>
	import Row from './Row.svelte';
	import Controls from './Controls.svelte';

	import {initAudio, playRow, startRecording, stopRecording, setScale, scales} from './Music.svelte';
	let config = {
		playing: false,
		speed: 175,
		rows: 12,
		scale_key: 'classic',
	}

	let columns = 36;
	let grid = [];
	let gameInterval;
	let curRow = 0;
	let lastRow = 0;
	let started = false;
	let downloadLink;
	let recording = false;

	const togglePlaying = async () => {
		config.playing = !config.playing;
		if(!started) {
			startPlaying();
		}
	}

	const startPlaying = async () => {
		if(!started) {
			config.playing = true;
			started = true;
			await initAudio();
		}
	}

	const stopPlaying = () => {
		config.playing = false;
		if(lastRow < grid.length) {
			grid[lastRow].isPlaying = false;
		}
		if(curRow < grid.length) {
			grid[curRow].isPlaying = false;
		}
		curRow = 0;
		lastRow = 0;
	}

	const clearGrid = (rows) => {
		curRow = 0;
		grid = [...Array(rows)].map(
			x => Array(columns).fill(false) );
	}

	const resizeGrid = (rows) => {
		stopPlaying();
		while(grid.length < rows) {
			grid.push([...Array(columns).fill(false)]);
		}

		while(grid.length > rows) {
			grid.pop();
		}

		grid[0].isPlaying = false;
	}

	const initGrid = (hash) => {
		config.playing = false;
		let array = hash.split('&')[0].slice(1).split('-').map(x => parseInt(x, 10));
		config.rows = array.length - 1;
		grid = []
		// add header note

		console.log(document.getElementById('notesContainer'));
		for (var i = array.length - 2; i >= 0; i--) {
			let temp = [... Array(columns).fill(false)];
			for (var j = columns - 1; j >= 0; j--) {
				temp[j] = (array[i] & (1 << (j))) !== 0;
			}
			grid.push(temp.reverse());
		}
		if(hash.split('&').length > 1) {
			config.speed = parseInt(hash.split('&')[1], 10);
		}
		if(hash.split('&').length > 2) {
			config.scale_key = hash.split('&')[2];
		}
	}

	const changeSpeed = (bpm) => {
		clearInterval(gameInterval);
		gameInterval = setInterval(() => {
			if(config.playing) {
				grid[lastRow].isPlaying = false;
				grid[curRow].isPlaying = true;
				playRow(grid[curRow]);
				lastRow  = curRow;
				curRow = (curRow + 1) % grid.length;
			}
		},  60 * 1000 / bpm);
	}

	const downloadAudio = () => {
		if(recording) {
			return;
		}

		recording = true;
		stopPlaying();
		config.playing = true;
		startRecording(downloadLink);

		// Play to completion
		let playbackTime = (grid.length * 60 * 1000) / config.speed;
		setTimeout(() => {
			stopPlaying();
		}, playbackTime);

		// Record for a bit more.
		setTimeout(() => {
			stopRecording();
			recording = false;
		}, playbackTime + 500);
	}

	$: changeSpeed(config.speed);

	clearGrid(config.rows);

	if(window.location.hash !== '') {
		initGrid(window.location.hash);
	}
</script>
<style>
	table {
		background: black;
	}

	.container {
		color: #ddd;
	}

	.footer a{
		font-size: 0.8em;
		color: #ccc;
		text-decoration: underline;
	}

	.tagline {
		font-size: 0.8em;
		font-weight: 600;
	}

	.message {
		font-size: 0.7em;
		color: red;
	}

	input[type=range] {
		width: 20em;
	}
</style>


<div class="container" align="center">
	<h3 style="text-decoration: underline;">Synthétiseur Music maker!</h3>
	<Controls
		bind:grid={grid}
		bind:config={config}
		on:playpause={togglePlaying}
		on:stop={stopPlaying}
		on:clear={() => clearGrid(config.rows)}
		on:rowchange={() => resizeGrid(config.rows)}
		on:scalechange={() => setScale(config.scale_key)}
		on:download={downloadAudio}
	/>
	{#if recording}
		<span class="message">Please wait for the playback to finish</span>
	{/if}
	<a bind:this={downloadLink} download="music-grid.wav" hidden="true">Download</a>
	<div id="notesContainer"></div>
	<table on:click|once={startPlaying}>
		{#each grid as row}
			<Row bind:row={row} bind:playing={row.isPlaying} paused={!config.playing}/>
		{/each}
	</table>
	<br/>
	<div class="footer" align="center">
		<a href="https://irshadpi.me/best-of-music-grid" target="_blank">Exemples de musiques</a>
		<br/>
		<a href="#0-2048-0-144-1072-0-40-530-0-268-24-0-1048-0-24-2072-0-2048-0-1024-0-0-512-18-0-268-24-0-1048-0-24-2072-0-0-2048-144-1072-0-40-530-0-268-24-0-1048-0-24-24-0-0-0-528-40-64-0-136-0-34-68-0-9-18-0-72-656-0-656-40-0-72-0-144-0-0-544-0-0-272-0-0-36-0-0-0-2304-0-0-0-17-0-1058-2084-0-1058-0-1058-1058-&399&classic" target="_blank">Mario</a> 
	</div>
	<br/>
	<br/>
</div>
