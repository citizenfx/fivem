import React from "react";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { BrandIcon } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";
import { useLocation } from "react-router-dom";
import { clsx } from "cfx/utils/clsx";
import { CurrentGameName } from "cfx/base/gameName";
import s from './HomeButton.module.scss';

const titles = [
  'Home',
  'Home Home',
  'Home Home Home',
  'Can you stop?',
  'Please',
  "This won't help",
  'Seriously',
  'Home',
  "This didn't quite work, right?",
  "To the beginning",
  "Home",
  "Wait, no",
  "Oh yea, press me more",
  "Now for real, to the beginning",
];

export const HomeButton = observer(function HomeButton() {
  const [index, setIndex] = React.useState(0);

  const resetRef = React.useRef(undefined);

  const handleClick = () => {
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
    }, 2000) as any;
  };

  const location = useLocation();

  React.useEffect(() => {
    if (resetRef.current) {
      clearTimeout(resetRef.current);
    }

    setIndex(0);
  }, [location.pathname]);

  return (
    <Title fixedOn="bottom-left" title={titles[index]}>
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
