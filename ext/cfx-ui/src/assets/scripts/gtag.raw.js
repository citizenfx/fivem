(function(w, d, targetGame) {
  var id = targetGame === 'rdr3' ? 'G-VD6XEVWWM3' : 'G-F4SNDFPX89';

  w.dataLayer = w.dataLayer || [];
  w.dataLayer.push('js', new Date());
  w.dataLayer.push('config', id);

  var f = d.getElementsByTagName('script')[0];
  var j = d.createElement('script');
  j.async = true;
  j.src = 'https://www.googletagmanager.com/gtag/js?id=' + id;
  f.parentNode.insertBefore(j, f);
})(window, document, nuiTargetGame);
