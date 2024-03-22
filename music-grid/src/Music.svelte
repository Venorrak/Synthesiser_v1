<script context="module">
  import * as Tone from "tone";
  import bufferToWav from "audiobuffer-to-wav";

  import { setSoundEnabled } from "./store.js";

  // update sound state
  let isSynthEnabled;
  setSoundEnabled.subscribe((value) => {
    console.log("MUSIC.SVELTE: Sound is now " + value);
    isSynthEnabled = value;
  });

  let synth;
  let recorder;
  let destinationStream;
  // midi notes for classic scale
  const midiNotes = {
    C1: 24,
    D1: 26,
    E1: 28,
    F1: 29,
    G1: 31,
    A1: 33,
    B1: 35,
    C2: 36,
    D2: 38,
    E2: 40,
    F2: 41,
    G2: 43,
    A2: 45,
    B2: 47,
    C3: 48,
    D3: 50,
    E3: 52,
    F3: 53,
    G3: 55,
    A3: 57,
    B3: 59,
    C4: 60,
    D4: 62,
    E4: 64,
    F4: 65,
    G4: 67,
    A4: 69,
    B4: 71,
    C5: 72,
    D5: 74,
    E5: 76,
    F5: 77,
    G5: 79,
    A5: 81,
    B6: 83,
    C6: 84,
  };

  // scales for the synth
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
  let midiNotesConversion = midiNotes;
  const sendMidiToSynth = true;

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

    // get notes to play
    let notesToPlay = [];
    let midiNotes = [];

    for (var i = row.length - 1; i >= 0; i--) {
      if (row[i]) {
        notesToPlay.push(currentScale[i]);
      }
    }

    // get midi notes to play
    for (var i = 0; i <= notesToPlay.length - 1; i++) {
      midiNotes.push(midiNotesConversion[notesToPlay[i]]);
    }
    //console.log(notesToPlay);
    //console.log(midiNotes);

    async function sendData(notes) {
      try {
        const response = await fetch("http://192.168.4.1", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            'Host' : '192.168.4.1'
          },
          body: JSON.stringify(notes),
        });

        if (response.ok) {
          console.log("Data sent successfully");
        } else {
          console.error("Failed to send data");
        }
      } catch (error) {
        console.error("Error:", error);
      }
    }

    //send midi notes to synth
    if (sendMidiToSynth && midiNotes.length > 0) {
      //send midi over wifi to 192.168.0.4
      console.log("Sending midi notes to synth: ", midiNotes);
      sendData(midiNotes);
    }

    // play notes sound in browser if enabled
    if (isSynthEnabled) {
      synth.triggerAttackRelease(notesToPlay, "16n");
    }

    // get playing row
    const playingRows = document.getElementsByClassName("playing");

    // if there are playing rows, scroll to it
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
    if (isSynthEnabled) {
      synth.triggerAttackRelease(currentScale[index], "16n");
    }
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
