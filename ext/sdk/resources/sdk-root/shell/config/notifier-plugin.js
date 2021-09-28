const http = require('http');
const execSync = require('child_process').execSync;

let port;
let server;
let sendEvent;

class NotifierPlugin {
  get client() {
    if (!port) {
      return '';
    }

    return `
    <script>
      (() => {
        const es = new EventSource('http://localhost:${port}/');
        es.onmessage = (event) => {
          window.location.reload();
        };
      })();
    </script>
    `;
  }

  apply(compiler) {
    compiler.hooks.done.tap('Notifier Plugin', () => {
      sendEvent?.('yay');
    });

    // compiler.hooks.watchRun.tap('Notifier Plugin', (compiler) => {
    //   console.log('BEFORE RUN', compiler.modifiedFiles);
    // });
  }
}

NotifierPlugin.init = () => {
  port = new FindFreePortSync({ start: 35500 }).getPort();

  server = http.createServer((_req, res) => {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive',
      'Access-Control-Allow-Origin': '*',
    });

    sendEvent = (data) => {
      res.write([
        `retry: 1000`,
        `id: ${Date.now()}`,
        `data: ${data}`,
        '',
        '',
      ].join('\n'));
    };
  });

  server.listen(port, () => {
    // console.log('NotifierPlugin server port:', port);
  });
};

module.exports = NotifierPlugin;

// copyied from https://github.com/imwtr/find-free-port-sync/blob/master/find-free-port-sync.js
/**
 * Finding free port synchronously
 * @param {Object} options [description]
 */
 function FindFreePortSync(options = {}) {
  this.defaultOptions = {
      // port start for scan
      start: 1,
      // port end for scan
      end: 65534,
      // ports number for scan
      num: 1,
      // specify ip for scan
      ip: '0.0.0.0|127.0.0.1',
      // for inner usage, some platforms like darkwin shows commom address 0.0.0.0:10000 as *.10000
      _ipSpecial: '\\*|127.0.0.1',
      // scan this port
      port: null
  };

  this.msg = {
      error: 'Cannot find free port'
  }

  this.adjustOptions(options);
}

FindFreePortSync.prototype = {
  constructor: FindFreePortSync,

  /**
   * Produce a correct options
   * @param  {[type]} options [description]
   * @return {[type]}         [description]
   */
  adjustOptions(options) {
      if (typeof options.start !== 'undefined') {
          options.start = parseInt(options.start, 10);
          options.start = (isNaN(options.start)
              || options.start < this.defaultOptions.start
              || options.start > this.defaultOptions.end
          ) ? this.defaultOptions.start : options.start;
      }

      if (typeof options.end !== 'undefined') {
          options.end = parseInt(options.end, 10);
          options.end = (isNaN(options.end)
              || options.end < this.defaultOptions.start
              || options.end > this.defaultOptions.end
          ) ? this.defaultOptions.end : options.end;
      }

      if (options.start > options.end) {
          let temp = options.start;
          options.start = options.end;
          options.end = temp;
      }

      options.num = isNaN(parseInt(options.num)) ? this.defaultOptions.num : parseInt(options.num);

      options._ipSpecial = options.ip ? options.ip : this.defaultOptions._ipSpecial;

      Object.assign(this, this.defaultOptions, options);
  },

  /**
   * Get random number
   * @return {[type]} [description]
   */
  getRandomPort() {
      return Math.floor(Math.random() * (this.end - this.start) + this.start);
  },

  /**
   * Find free port
   * @return {[type]} [description]
   */
  getPort() {
      let stepIndex = 0,
          maxStep = 65536,
          freePort = null,
          res = '',
          portSplitStr = ':',
          reg = new RegExp(`\\s(${this.ip}):(\\d+)\\s`, 'g'),
          regSpecial = new RegExp(`\\s(${this._ipSpecial})\\.(\\d+)\\s`, 'g');

      try {
          // get network state list
          res = execSync('netstat -an', {
              encoding: 'utf-8'
          });

          let usedPorts = res.match(reg);

          // special address usage for  ip.port
          if (!usedPorts) {
              usedPorts = res.match(regSpecial);
              portSplitStr = '.';
          }

          usedPorts = !usedPorts ? [] : usedPorts.map(item => {
              let port = item.split(portSplitStr);
              port = port.slice(-1)[0];
              return parseInt(port.slice(0, -1), 10);
          });

          // check the port if usage and return directly
          if (this.port) {
              return !usedPorts.includes(this.port)
                  ? false
                  : (this.port >= this.start && this.port <= this.end);
          }

          usedPorts = [...new Set(usedPorts)];

          // get a random free port
          if (this.num === 1) {
              let portAvaliable = false;

              while (!portAvaliable) {
                  freePort = this.getRandomPort();

                  if (!usedPorts.includes(freePort)) {
                      portAvaliable = true;
                  }

                  if (++stepIndex > maxStep) {
                      console.log(this.msg.error);
                  }

                  return freePort;
              }
          }

          // return free ports orderly
          freePort = [];

          for (let i = this.end, n = 1; i >= this.start; --i) {
              if (!usedPorts.includes(i)) {
                  if (n++ > this.num) {
                      continue;
                  }

                  freePort.push(i);
              }
          }

          if (!freePort.length) {
              console.log(this.msg.error);
          }

          return freePort;

      } catch(e) {
          console.log(this.msg.error);
          console.log(e);
      }

      return freePort;
  }
}
