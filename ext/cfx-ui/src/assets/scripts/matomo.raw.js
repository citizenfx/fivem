var _paq = _paq || [];
_paq.push(['enableLinkTracking']);
(function () {
  var u = "https://metrics.fivem.net/";
  _paq.push(['setTrackerUrl', u + 'piwik.php']);
  _paq.push(['setSiteId', (window.invokeNative) ? '3' : '4']);
  _paq.push(['setCustomDimension', customDimensionId = 1, customDimensionValue = getGPU()]);
  _paq.push(['setCustomDimension', customDimensionId = 2, customDimensionValue = navigator.hardwareConcurrency]);
  _paq.push(['setCookieSameSite', 'None']);
  _paq.push(['setSecureCookie', true]);

  var matomoScriptUrl = u + 'piwik.js?2';

  if (globalThis.fetch) {
    // By prefetching the script we're unblocking the main thread
    fetch(matomoScriptUrl).then((response) => {
      if (response.ok) {
        return response.blob();
      }

      return false;
    }).then((blob) => {
      if (!blob) {
        return;
      }

      run(URL.createObjectURL(blob));
    });
  } else {
    run(matomoScriptUrl);
  }

  function run(src) {
    var d = document, g = d.createElement('script'), s = d.getElementsByTagName('script')[0];
    g.type = 'text/javascript'; g.async = true; g.defer = true; g.src = src; s.parentNode.insertBefore(g, s);
  }

  function getGPU() {
    try {
      const gl = document.createElement('canvas').getContext('webgl');
      const debug = gl.getExtension('WEBGL_debug_renderer_info');

      return debug ? gl.getParameter(debug.UNMASKED_RENDERER_WEBGL) : '';
    } catch (e) {
      return '';
    }
  }
})();
