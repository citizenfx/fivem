import React from 'react';
import { observer } from 'mobx-react-lite';
import { BaseTool } from '../BaseTool/BaseTool';
import { WETool } from '../../../store/WEToolbarState';
import { WEATHER, WEEnvState } from 'personalities/world-editor/store/WEEnvState';
import { BiSun } from 'react-icons/bi';
import { BsClock } from 'react-icons/bs';
import s from './EnvironmentTool.module.scss';
import { IntroForceRecalculate } from 'fxdk/ui/Intro/Intro';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';

export const EnvironmentTool = observer(function EnvironmentTool() {
  const weatherNodes = Object.keys(WEATHER).map((weather) => (
    <button
      key={weather}
      className="normalize-text"
      onClick={() => WEEnvState.setWeather(weather as any)}
    >
      {weather}
    </button>
  ));

  return (
    <BaseTool
      renderAlways
      tool={WETool.Environment}
      icon={<BiSun />}
      label="Environment"

      toggleProps={{ 'data-intro-id': 'environment-panel' }}
      panelProps={{ 'data-intro-id': 'environment-panel' }}
    >
      <IntroForceRecalculate />

      <div className={s.root}>
        <div className={s['clock-header']}>
          <div className={s.entry}>
            <BsClock /> {WEEnvState.time}
          </div>
          <Checkbox
            value={WEEnvState.freezeTime}
            onChange={(freeze) => WEEnvState.freezeTime = freeze}
            label="Freeze"
          />
        </div>

        <div className={s.clock}>
          <input
            type="range"
            min={1}
            max={60 * 24 - 1}
            value={WEEnvState.timeNum}
            onChange={({ target: { value } }) => WEEnvState.setTimeNum(value)}
          />
        </div>

        <div className={s.entry}>
          <BiSun />
          <span className="normalize-text">
            {
              WEEnvState.prevWeather === WEEnvState.nextWeather
                ? WEEnvState.prevWeather
                : `${WEEnvState.prevWeather} -> ${WEEnvState.nextWeather}`
            }
          </span>
        </div>

        <div className={s['weather-buttons']}>
          <button onClick={() => WEEnvState.setRandomWeather()}>
            Dynamic weather
          </button>

          {weatherNodes}
        </div>
      </div>
    </BaseTool>
  );
});
