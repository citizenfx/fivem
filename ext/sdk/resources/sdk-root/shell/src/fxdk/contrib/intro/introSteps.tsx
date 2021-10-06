import React from 'react';
import { IntroStep } from 'fxdk/ui/Intro/Intro';

export const introTourSteps: IntroStep[] = [
  {
    backButtonDisabled: true,
    content() {
      return (
        <>
          <h1>
            Welcome to FxDK
          </h1>

          <p>
            FxDK is the developer tool for FiveM.
            Create servers and resources with ease.
            Export and build all the files you will need to run a server when you are done.
          </p>
        </>
      );
    },
  },
  {
    focusElement: 'create-project-item',
    content() {
      return (
        <>
          <h3>
            Creativity starts here
          </h3>

          <p>
            Either create something new or import existing stuff.
          </p>
        </>
      );
    },
  },
  {
    focusElement: 'project-explorer',
    content() {
      return (
        <>
          <h3>Project Explorer</h3>

          <p>
            This is the outline of your project: all files, directories and resources live in here.
          </p>
        </>
      );
    },
  },
  {
    focusElement: 'start-server-button',
    content() {
      return (
        <>
          <h3>
            Start server button
          </h3>

          <p>
            Whenever you ready to test your creation, hit this button.
          </p>

          <p>
            It will start the dev server and open <strong>Game</strong> view tab in editor,
            it may take some time to load though, be patient!
          </p>

          <p>
            Once server has started - hit it again to stop the dev server.
          </p>
        </>
      );
    },
  },
  {
    focusElement: 'fxcode',
    content() {
      return (
        <>
          <h3>FXCode, the editor</h3>

          <p>
            This is the main area where you can editor your project files, <strong>Game</strong> view tab also lives here.
          </p>

          <p>
            Are you familiar with Microsoft's VSCode? If so, you know FXCode already, other than that, we have:

            <ul>
              <li>Game view</li>
              <li>Resource monitor</li>
              <li>Both game and server consoles</li>
            </ul>
          </p>
        </>
      );
    },
  },
  {
    focusElement: 'project-build',
    content() {
      return (
        <>
          <h3>Project building</h3>

          <p>
            When your creation is shaped nicely, it's time to build it!
          </p>

          <p>
            FxDK will build your resources and configuration files and place ready-to-run server under selected build path.
          </p>

          <p>
            All you have to do next is add your license key in there and deploy it on your server.
          </p>
        </>
      );
    },
  },
  {
    content() {
      return (
        <>
          <h3>Last, but not least</h3>

          <p>
            It's not all possibilities FxDK can offer, so feel free to explore FxDK more!
          </p>

          <p>
            If you encountered a bug - please leave
            a detailed report on forums under the <a href="https://forum.cfx.re/c/fxdk/fxdk-feedback/73">FxDK feedback section</a>.
          </p>

          <p>
            Get in touch with other users of FxDK in our <a href="https://dicord.gg/fivem">discord guild</a> in <strong>#fxdk-discussion</strong> channel.
          </p>
        </>
      );
    },
    nextButtonText: 'Get started!',
  }
];
