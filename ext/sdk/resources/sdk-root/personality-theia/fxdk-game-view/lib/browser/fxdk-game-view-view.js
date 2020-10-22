"use strict";
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
var FxdkGameView_1, FxdkGameViewContribution_1;
Object.defineProperty(exports, "__esModule", { value: true });
exports.FxdkGameViewContribution = exports.FxdkGameView = void 0;
const React = require("react");
const browser_1 = require("@theia/core/lib/browser");
const browser_2 = require("@theia/core/lib/browser");
const inversify_1 = require("inversify");
require("./common/game-view.webcomponent.js");
const stylesheet = /*css*/ `
  @keyframes hint-animation {
    0% {
      opacity: 1;
    }
    75% {
      opacity: 1;
    }
    100% {
      opacity: 0;
    }
  }

  .fxdk-game-view {
    display: flex;
    flex-direction: column;
    width: 100%;
    height: 100%;
    outline: none;
  }
  .fxdk-game-view:hover .fxdk-play-button {
    transform: translateY(0);
  }

  .fxdk-game-bar-wrapper {
    position: absolute;
    top: 0;
    left: 0;
    right: 0;

    display: flex;
    justify-content: center;

    z-index: 10;
  }

  .fxdk-game-bar {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
  }
  .fxdk-game-bar.fxdk-pointer-locked .fxdk-play-button {
    display: none;
  }

  .fxdk-play-button {
    padding: 5px 15px;

    backdrop-filter: blur(20px);
    background-color: rgba(255, 255, 255, .25);

    color: white;
    font-size: 14px;
    text-transform: uppercase;
    font-family: 'Segoe UI';
    font-weight: 500;
    letter-spacing: 1px;
    text-shadow: 0 0 2px rgba(0,0,0,.25);

    border: solid 1px rgba(255, 255, 255, .1);
    border-top: none;
    border-bottom-left-radius: 4px;
    border-bottom-right-radius: 4px;

    cursor: pointer;
    outline: none;

    transform: translateY(-100%);
    transition: transform .2s ease;
  }
  .fxdk-play-button:hover {
    background-color: rgba(255, 255, 255, .45);
  }

  .fxdk-game-view game-view {
    flex-grow: 1;
    width: 100%;
  }
`;
const GameView = React.memo(() => {
    const [pointerLocked, setPointerLocked] = React.useState(false);
    const gameViewRef = React.useRef(null);
    const enterFullscreen = React.useCallback(() => {
        if (gameViewRef.current) {
            gameViewRef.current.enterFullscreen();
            gameViewRef.current.lockPointer();
        }
    }, []);
    React.useEffect(() => {
        const pointerlockchangeCallback = (e) => {
            setPointerLocked(e.detail.locked);
        };
        if (gameViewRef.current) {
            gameViewRef.current.addEventListener('pointerlockchange', pointerlockchangeCallback);
        }
        return () => {
            if (gameViewRef.current) {
                gameViewRef.current.removeEventListener('pointerlockchange', pointerlockchangeCallback);
            }
        };
    }, []);
    return (React.createElement("div", { tabIndex: 0, className: "fxdk-game-view" },
        React.createElement("div", { className: "fxdk-game-bar-wrapper" },
            React.createElement("div", { className: `fxdk-game-bar ${pointerLocked ? 'fxdk-pointer-locked' : ''}` },
                React.createElement("button", { onClick: enterFullscreen, className: "fxdk-play-button" }, "Go fullscreen"))),
        React.createElement("game-view", { ref: gameViewRef })));
});
let FxdkGameView = FxdkGameView_1 = class FxdkGameView extends browser_1.ReactWidget {
    init() {
        this.id = FxdkGameView_1.ID;
        this.title.closable = true;
        this.title.caption = 'Game view';
        this.title.label = 'Game view';
        this.title.iconClass = 'fa fa-gamepad';
        this.update();
    }
    onActivateRequest() {
        this.node.focus();
    }
    render() {
        return (React.createElement(React.Fragment, null,
            React.createElement("style", null, stylesheet),
            React.createElement(GameView, null)));
    }
};
FxdkGameView.ID = 'fxdkGameView';
__decorate([
    inversify_1.postConstruct(),
    __metadata("design:type", Function),
    __metadata("design:paramtypes", []),
    __metadata("design:returntype", void 0)
], FxdkGameView.prototype, "init", null);
FxdkGameView = FxdkGameView_1 = __decorate([
    inversify_1.injectable()
], FxdkGameView);
exports.FxdkGameView = FxdkGameView;
let FxdkGameViewContribution = FxdkGameViewContribution_1 = class FxdkGameViewContribution extends browser_2.AbstractViewContribution {
    constructor() {
        super({
            widgetId: FxdkGameView.ID,
            widgetName: 'Game View',
            toggleCommandId: FxdkGameViewContribution_1.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID,
            defaultWidgetOptions: {
                area: 'main',
            }
        });
    }
};
FxdkGameViewContribution.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID = 'fxdkGameView:toggle';
FxdkGameViewContribution = FxdkGameViewContribution_1 = __decorate([
    inversify_1.injectable(),
    __metadata("design:paramtypes", [])
], FxdkGameViewContribution);
exports.FxdkGameViewContribution = FxdkGameViewContribution;
//# sourceMappingURL=fxdk-game-view-view.js.map