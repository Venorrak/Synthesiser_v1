<script context="module">
  import * as Tone from "tone";
  import bufferToWav from "audiobuffer-to-wav";

  let synth;
  let recorder;
  let destinationStream;

  let sound = true; //enable 

  export const scales = {
    classic: [
      "C1",
      "D1",
      "E1",
      "F1",
      "G1",
      "A1",
      "B1",
      "C2",
      "D2",
      "E2",
      "F2",
      "G2",
      "A2",
      "B2",
      "C3",
      "D3",
      "E3",
      "F3",
      "G3",
      "A3",
      "B3",
      "C4",
      "D4",
      "E4",
      "F4",
      "G4",
      "A4",
      "B4",
      "C5",
      "D5",
      "E5",
      "F5",
      "G5",
      "A5",
      "B6",
      "C6",
    ],
  };

  let currentScale = scales["classic"];

  export const scale_keys = Object.keys(scales);

  export const initAudio = async () => {
    destinationStream = Tone.context.createMediaStreamDestination();

    synth = new Tone.PolySynth(4, Tone.Synth, {
      oscillator: {
        type: "triangle",
      },
    }).toMaster();
    synth.connect(destinationStream);

    synth.set("detune", -1200);

    await Tone.start();
    await Tone.context.resume();
    console.log("audio is ready");

  };

  export const setScale = (key) => {
    currentScale = scales[key];
  };

  export const playRow = async (row) => {
    if (!synth) {
      await initAudio();
    }

    let notesToPlay = [];
    for (var i = row.length - 1; i >= 0; i--) {
      if (row[i]) {
        notesToPlay.push(currentScale[i]);
      }
    }
    console.log(notesToPlay); // log the notes to play
    if (sound) {
      synth.triggerAttackRelease(notesToPlay, "16n");
    }

    // scroll to playing row
    const playingRows = document.getElementsByClassName("playing");

    if (playingRows.length > 0) {
      const playingRow = playingRows[0];

      playingRow.scrollIntoView({
        behavior: "smooth",
        block: "center",
      });
    }
  };

  export const playCell = async (index) => {
    if (!synth) {
      await initAudio();
    }
    synth.triggerAttackRelease(currentScale[index], "16n");
  };

  export const stopRecording = async () => {
    if (recorder) {
      recorder.stop();
    }
  };

  export const startRecording = async (downloadLink) => {
    if (!destinationStream) {
      await initAudio();
    }

    recorder = new MediaRecorder(destinationStream.stream);

    let audioChunks = [];
    recorder.ondataavailable = async (e) => {
      let arrayBuffer = await e.data.arrayBuffer();
      Tone.context.decodeAudioData(arrayBuffer, (buffer) => {
        let wav = bufferToWav(buffer);
        let blob = new Blob([new DataView(wav)], {
          type: "audio/wav",
        });
        downloadLink.href = URL.createObjectURL(blob);
        downloadLink.click();
      });
    };
    recorder.start();
  };
</script>
