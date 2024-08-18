import * as forge from 'node-forge';

import { IRSAKeys } from './types';

let keys: IRSAKeys | null = null;
const workerScript = new URL('node-forge/dist/prime.worker.min.js', import.meta.url).toString();

try {
  const savedKeysString = window.localStorage.getItem('rsaKeys');

  if (savedKeysString) {
    keys = JSON.parse(savedKeysString);
  }
} catch (e) {
  // noop
}

export async function getOrCreateRSAKeys(): Promise<IRSAKeys> {
  if (!keys) {
    keys = await generateRSAKeys();
  }

  return keys;
}

export async function decryptBase64(payload: string): Promise<string> {
  const rsaKeys = await getOrCreateRSAKeys();
  const pkey = forge.pki.privateKeyFromPem(rsaKeys.private);

  return pkey.decrypt(forge.util.decode64(payload));
}

async function generateRSAKeys(): Promise<IRSAKeys> {
  const rsaKeys = await new Promise<IRSAKeys>((resolve, reject) => {
    forge.pki.rsa.generateKeyPair(
      {
        bits: 2048,
        workers: -1,
        workerScript,
      },
      (err, keypair) => {
        if (err) {
          reject(err);

          return;
        }

        resolve({
          public: forge.pki.publicKeyToPem(keypair.publicKey),
          private: forge.pki.privateKeyToPem(keypair.privateKey),
        });
      },
    );
  });

  window.localStorage.setItem('rsaKeys', JSON.stringify(rsaKeys));

  return rsaKeys;
}

try {
  (window as any).__generateRSAKeys = generateRSAKeys;
} catch {
  // Do nothing
}
