import { encode, decode } from 'blurhash';

const WIDTH = 64;
const HEIGHT = 64;
const COMP_X = 9;
const COMP_Y = 9;

onmessage = (event: MessageEvent) => createBlurred(event.data);

async function createBlurred(imageUrl: string) {
  console.time('BackdropBlur process');

  const [pixels, width, height] = getImageData(await loadImage(imageUrl));

  const blurhash = decode(encode(pixels, width, height, COMP_X, COMP_Y), WIDTH, HEIGHT);
  const cvs = new OffscreenCanvas(WIDTH, HEIGHT);
  const ctx = cvs.getContext('2d');

  const bhImageData = ctx!.createImageData(WIDTH, HEIGHT);
  bhImageData.data.set(blurhash);

  ctx!.putImageData(bhImageData, 0, 0);

  let url: string | null = null;

  try {
    const blob = await cvs.convertToBlob();
    url = URL.createObjectURL(blob);
  } catch (e) {
    console.error(e);
  }

  console.timeEnd('BackdropBlur process');

  postMessage(url);
}

async function loadImage(src: string) {
  const img = await (
    await fetch(src, {
      mode: 'no-cors',
    })
  ).blob();

  return createImageBitmap(img);
}

function getImageData(image: HTMLImageElement | ImageBitmap): [Uint8ClampedArray, number, number] {
  const ar = image.width / image.height;

  const w = 128;
  // eslint-disable-next-line no-bitwise
  const h = w * ar | 0;

  const canvas = new OffscreenCanvas(w, h);

  const context = canvas.getContext('2d');
  context!.drawImage(image, 0, 0, w, h);

  return [context!.getImageData(0, 0, w, h).data, w, h];
}
