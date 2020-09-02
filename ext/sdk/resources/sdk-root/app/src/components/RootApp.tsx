import React from 'react';
import { sendCommand } from '../utils/sendCommand';
import { GameViewMode, GameView } from './common/GameView';

const startSDKGame = () => sendCommand('localgame sdk-game');
const restartSDKGame = () => sendCommand('localrestart');

const giveRifle = () => sendCommand('weapon WEAPON_CARBINERIFLE');

export function RootApp() {
  const [mode, setMode] = React.useState(GameViewMode.observing);
  const [coords, setCoords] = React.useState(null);

  React.useEffect(() => {
    const messageRcv = (e: MessageEvent) => {
      if (e.data.type === 'pos') {
        setCoords(e.data.payload);
      }
    };

    window.addEventListener('message', messageRcv);

    return () => window.removeEventListener('message', messageRcv);
  }, []);

  return (
    <>
      <button onClick={() => window.openDevTools()}>Open DevTools</button>

      <button onClick={startSDKGame}>localgame sdk-game</button>
      <button onClick={restartSDKGame}>localrestart sdk-game</button>

      <button onClick={giveRifle}>give rifle</button>

      <br/>
      <button onClick={() => setMode(GameViewMode.observing)}>observing mode</button>
      <button onClick={() => setMode(GameViewMode.controling)}>controling mode</button>
      <br/>

      <code>Mode: {GameViewMode[mode]}</code>
      <br/>
      <code>Pos: {JSON.stringify(coords, null, 2)}</code>

      <GameView mode={mode}/>
    </>
  );
}
