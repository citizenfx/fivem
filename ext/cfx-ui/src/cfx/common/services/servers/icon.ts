import { createPlaceholderIconDataURI } from "cfx/base/placeholderIcon";

export function getServerIconURL(address: string, version?: string | number | null): string {
  if (!version) {
    return createPlaceholderIconDataURI(address);
  }

  return `https://servers-frontend.fivem.net/api/servers/icon/${address}/${version}.png`;
}

export async function getServerIconThumbnailURL(address: string, version?: string | number | null): Promise<string> {
  return createThumbnail(getServerIconURL(address, version));
}

async function createThumbnail(uri: string, width = 16, height = 16): Promise<string> {
  try {
    const res = await window.fetch(uri);
    if (!res.ok) {
      return '';
    }

    const bitmap: ImageBitmap = await createImageBitmap(await res.blob(), {
      resizeQuality: 'medium',
      resizeWidth: width,
      resizeHeight: height,
    });

    const canvas = document.createElement('canvas');
    canvas.width = bitmap.width;
    canvas.height = bitmap.height;

    const context = canvas.getContext('2d');
    if (!context) {
      return '';
    }

    context.drawImage(bitmap, 0, 0);

    const outBlob = await new Promise<Blob | null>((resolve) => canvas.toBlob(resolve, 'image/png'));
    if (!outBlob) {
      return '';
    }

    return new Promise((resolve) => {
      const fileReader = new FileReader();

      fileReader.addEventListener('load', () => {
        resolve(fileReader.result as string);
      });

      fileReader.readAsDataURL(outBlob);
    });
  } catch (e) {
    return '';
  }
}
