import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    proxy: {
      '/api': {
        target: 'http://localhost:3000',
        changeOrigin: true,
        configure: (proxy, options) => {
          proxy.on('error', (err, _req, _res) => {
            console.log('API Proxy error:', err);
          });
        }
      },
      '/ws': {
        target: 'ws://localhost:3000',
        ws: true,
        configure: (proxy, options) => {
          // Handle proxy errors
          proxy.on('error', (err, _req, _res) => {
            if (err.code === 'EPIPE' || err.code === 'ECONNRESET') return;
            console.log('WS Proxy error:', err);
          });
          
          // Handle socket errors specifically for WebSockets
          proxy.on('proxyReqWs', (proxyReq, req, socket, options, head) => {
            socket.on('error', (err) => {
              if (err.code === 'EPIPE' || err.code === 'ECONNRESET') return;
              console.log('WS Socket error:', err);
            });
          });
        }
      }
    }
  },
  build: {
    outDir: 'dist',
    sourcemap: true
  }
})
