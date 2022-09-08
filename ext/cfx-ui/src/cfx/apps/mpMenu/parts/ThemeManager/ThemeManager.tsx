import React from "react";
import { observer } from "mobx-react-lite";
import { getCustomInGameBackdropPath } from "cfx/apps/mpMenu/parts/ThemeManager/backdrop";
import { CurrentGameName } from "cfx/base/gameName";
import { useService } from "cfx/base/servicesContainer";
import { IConvarService } from "../../services/convars/convars.service";
import { BackdropBlurWorker } from "./BackdropBlur";

export const ThemeManager = observer(function ThemeManager() {
  const ConvarService = useService(IConvarService);

  // Apply custom backdrop image
  const backdropPathRaw = ConvarService.get('ui_customBackdrop');
  const backdropPath = React.useMemo(() => {
    if (!backdropPathRaw) {
      return '';
    }

    return getCustomInGameBackdropPath(backdropPathRaw);
  }, [backdropPathRaw]);
  React.useEffect(() => {
    BackdropBlurWorker.setUrl(backdropPath);

    if (backdropPath) {
      document.body.style.setProperty('--backdrop-image', `url(${backdropPath})`);
    } else {
      document.body.style.removeProperty('--backdrop-image');
    }
  }, [backdropPath]);
  const backdropBlurPath = BackdropBlurWorker.url;
  React.useEffect(() => {
    if (backdropBlurPath) {
      document.body.style.setProperty('--backdrop-image-blur', `url(${backdropBlurPath})`);
    } else {
      document.body.style.removeProperty('--backdrop-image-blur');
    }
  }, [backdropBlurPath]);

  // Theming
  const lastThemeClassNameRef = React.useRef('');
  const preferLightColorScheme = ConvarService.getBoolean('ui_preferLightColorScheme');
  React.useEffect(() => {
    const themeClassName = `cfxui-theme-${CurrentGameName}-${preferLightColorScheme ? 'light' : 'dark'}`;

    if (lastThemeClassNameRef.current) {
      document.body.classList.remove(lastThemeClassNameRef.current);
    }

    lastThemeClassNameRef.current = themeClassName;

    document.body.classList.add(themeClassName);
  }, [preferLightColorScheme]);

  // Blur
  // NOTHING TO ACTUALLY CONTROL BY THIS RIGHT NOW
  // const backdropBlurReduction = ConvarService.get('ui_blurPerfMode');
  // React.useEffect(() => {
  //   switch (backdropBlurReduction) {
  //     case 'off': {
  //       document.body.classList.remove('reduce-backdrop-blur');
  //       document.body.classList.remove('no-backdrop-blur');
  //       break;
  //     }
  //     case 'backdrop': {
  //       document.body.classList.remove('no-backdrop-blur');
  //       document.body.classList.add('reduce-backdrop-blur');
  //       break;
  //     }
  //     case 'none': {
  //       document.body.classList.remove('reduce-backdrop-blur');
  //       document.body.classList.add('no-backdrop-blur');
  //       break;
  //     }
  //   }
  // }, [backdropBlurReduction]);

  return null;
});
