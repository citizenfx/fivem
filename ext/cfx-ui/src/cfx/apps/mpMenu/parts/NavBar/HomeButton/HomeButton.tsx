import { BrandIcon, Title, clsx } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useLocation } from 'react-router-dom';

import { CurrentGameName } from 'cfx/base/gameRuntime';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { LinkButton } from 'cfx/ui/Button/LinkButton';

import s from './HomeButton.module.scss';

const titles = [
  'Home', // Unused, we're using localized version for this one
  'Home Home',
  'Home Home Home',
  'Can you stop?',
  'Please',
  'This won\'t help',
  'Seriously',
  'Home',
  'This didn\'t quite work, right?',
  'To the beginning',
  'Home',
  'Wait, no',
  'Oh yea, press me more',
  'Now for real, to the beginning',
];

export const HomeButton = observer(function HomeButton() {
  const [index, setIndex] = React.useState(0);
  const eventHandler = useEventHandler();

  const resetRef = React.useRef<ReturnType<typeof setTimeout> | undefined>(undefined);

  const handleGTMSiteNavEvent = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.SiteNavClick,
      properties: {
        text: '#BottomNav_Home',
        link_url: '/',
        element_placement: ElementPlacements.Nav,
        position: 0,
      },
    });
  }, [eventHandler]);

  const handleClick = React.useCallback(() => {
    let newIndex = index + 1;

    if (newIndex > titles.length - 1) {
      newIndex = 0;
    }

    setIndex(newIndex);

    if (resetRef.current) {
      clearTimeout(resetRef.current);
    }

    resetRef.current = setTimeout(() => {
      setIndex(0);
    }, 2000);

    handleGTMSiteNavEvent();
  }, [index, handleGTMSiteNavEvent]);

  const location = useLocation();

  React.useEffect(() => {
    if (resetRef.current) {
      clearTimeout(resetRef.current);
    }

    setIndex(0);
  }, [location.pathname]);

  const title = index === 0
    ? $L('#BottomNav_Home')
    : titles[index];

  return (
    <Title fixedOn="bottom-left" title={title}>
      <LinkButton
        to="/"
        size="large"
        theme="transparent"
        icon={BrandIcon[CurrentGameName] || BrandIcon.cfxre}
        className={clsx(s.root, s.specific)}
        onClick={handleClick}
      />
    </Title>
  );
});
