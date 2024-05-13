import { Howl } from 'howler';

export enum Sfx {
  Click1 = 'c1',
  Click2 = 'c2',
  Click3 = 'c3',
  Click4 = 'c4',
  Woosh1 = 'w1',
  Woosh2 = 'w2',
  Woosh3 = 'w3',
  Woosh4 = 'w4',
  Connect = 'CONNECT_SOUND_EFFECT',
  Maybe = 'maybe',
}

const off0 = 0;
const off1 = 1710;
const off2 = 3420;
const off3 = 5140;
const off4 = 6850;
const off5 = 8570;
const off6 = 10280;
const off7 = 12000;
const off8 = 13710;
const off9 = 15460;

const sprite: Record<Sfx, [number, number]> = {
  [Sfx.Click1]: [off0, off1 - off0],
  [Sfx.Click2]: [off1, off2 - off1],
  [Sfx.Click3]: [off2, off3 - off2],
  [Sfx.Click4]: [off3, off4 - off3],
  [Sfx.Woosh1]: [off4, off5 - off4],
  [Sfx.Woosh2]: [off5, off6 - off5],
  [Sfx.Woosh3]: [off6, off7 - off6],
  [Sfx.Woosh4]: [off7, off8 - off7],
  [Sfx.Connect]: [off8, off9 - off8],
  [Sfx.Maybe]: [off9, 800],
};

const howl = new Howl({
  src: [new URL('assets/sounds/sfx.mp3', import.meta.url).toString()],
  sprite,
  volume: 2,
});

export function playSfx(sfx: Sfx) {
  try {
    howl.play(sfx);
  } catch (e) {
    // no-op
  }
}
