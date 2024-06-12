import { Deferred, idleCallback } from 'cfx/utils/async';

let loadingSplashShutdownRequested = true;

export const loadingSplashInactiveDeferred = new Deferred<void>();

export function shutdownLoadingSplash() {
  if (!loadingSplashShutdownRequested) {
    return;
  }

  loadingSplashShutdownRequested = false;

  try {
    const $loader = document.getElementById('loader');

    if (!$loader) {
      throw new Error('No #loader found, did it get deleted from index.html?');
    }

    const $loaderMask = $loader.querySelector('#loader-mask');

    if (!$loaderMask) {
      throw new Error('No #loader-mask found, did it get deleted from index.html?');
    }

    requestAnimationFrame(() => {
      $loader.classList.add('hide');

      $loaderMask.addEventListener('animationend', async () => {
        await idleCallback(1000);

        $loader.parentNode?.removeChild($loader);
      });

      loadingSplashInactiveDeferred.resolve();
    });
  } catch (e) {
    loadingSplashInactiveDeferred.resolve();

    console.error(e);
  }
}
