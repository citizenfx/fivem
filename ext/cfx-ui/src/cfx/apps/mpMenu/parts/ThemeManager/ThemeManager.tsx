import React from "react";
import { observer } from "mobx-react-lite";
import { getCustomInGameBackdropPath } from "cfx/apps/mpMenu/parts/ThemeManager/backdrop";
import { CurrentGameName } from "cfx/base/gameRuntime";
import { useService } from "cfx/base/servicesContainer";
import { IConvarService, KnownConvars } from "../../services/convars/convars.service";
import { BackdropBlurWorker } from "./BackdropBlur";

export const ThemeManager = observer(function ThemeManager() {
  const ConvarService = useService(IConvarService);

  // Apply custom backdrop image
  const backdropPathRaw = ConvarService.get(KnownConvars.customBackdrop);
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
  const preferLightColorScheme = ConvarService.getBoolean(KnownConvars.preferLightColorScheme);
  React.useEffect(() => {
    // const themeClassName = `cfxui-theme-${CurrentGameName}-${preferLightColorScheme ? 'light' : 'dark'}`;
    const themeClassName = `cfxui-theme-${CurrentGameName}-devoted`;

    if (lastThemeClassNameRef.current) {
      document.body.classList.remove(lastThemeClassNameRef.current);
    }

    lastThemeClassNameRef.current = themeClassName;

    document.body.classList.add(themeClassName);
  }, [preferLightColorScheme]);

  // Backdrop blur
  const preferBlurredBackdrop = ConvarService.getBoolean(KnownConvars.preferBlurredBackdrop);
  React.useEffect(() => {
    if (preferBlurredBackdrop) {
      document.body.classList.add('cfxui-blurred-backdrop');
    } else {
      document.body.classList.remove('cfxui-blurred-backdrop');
    }
  }, [preferBlurredBackdrop]);

  return null;
});
