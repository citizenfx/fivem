import React from 'react';
import { Steps } from 'intro.js-react';
import { introOptions, steps } from './TourSettings';
import 'intro.js/introjs.css';
import './Tour.module.scss';

interface TourProps {
  tourVisible: boolean,
  setTourVisible: (tourVisible: boolean) => void,
}

export const Tour = React.memo(function Tour({ tourVisible, setTourVisible }: TourProps) {
  const [stepsEnabled, setStepsEnabled] = React.useState(true);

  const onExit = () => {
    setStepsEnabled(false);
    setTimeout(() => {
      //allows you to open Tour again after close
      //prevents sentry error
      setTourVisible(false);
      setStepsEnabled(true);
    }, 500);
  };

  if (!tourVisible) {
    return null;
  }

  return (
    <Steps
      enabled={stepsEnabled}
      steps={steps}
      initialStep={0}
      onExit={onExit}
      options={introOptions}
    />
  );
});
