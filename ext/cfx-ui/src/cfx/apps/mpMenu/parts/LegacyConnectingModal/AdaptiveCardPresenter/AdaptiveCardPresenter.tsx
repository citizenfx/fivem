import React from "react";
import * as AC from "adaptivecards";
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { CurrentGameBrand } from "cfx/base/gameName";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Text } from "cfx/ui/Text/Text";
import { noop } from "cfx/utils/functional";
import { renderMarkdown } from "cfx/utils/markdown";
import { observer } from "mobx-react-lite";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { IUiService, useUiService } from "cfx/common/services/ui/ui.service";
import s from './AdaptiveCardPresenter.module.scss';

AC.AdaptiveCard.onProcessMarkdown = (text, result) => {
  result.outputHtml = renderMarkdown(text);
  result.didProcess = true;
};

export interface AdaptiveCardPresenterProps {
  card: string | AC.AdaptiveCard,

  onCancel?(): void,
}

export const AdaptiveCardPresenter = observer(function AdaptiveCardPresenter(props: AdaptiveCardPresenterProps) {
  const {
    card,
    onCancel = noop,
  } = props;

  const uiService = useUiService();

  const cardRef = React.useRef<AC.AdaptiveCard | null>(null);
  const containerRef = React.useRef<HTMLDivElement>(null);
  const submittingRef = React.useRef(false);

  const renderCard = React.useCallback(() => {
    if (!containerRef.current || !cardRef.current) {
      return;
    }

    cardRef.current.hostConfig = getHostConfig(uiService);
    containerRef.current.innerHTML = '';
    containerRef.current.appendChild(cardRef.current.render()!);
  }, []);

  React.useEffect(renderCard, [uiService.quant]);

  React.useEffect(() => {
    if (!containerRef.current) {
      return;
    }

    if (typeof card === 'string') {
      cardRef.current = new AC.AdaptiveCard();
      cardRef.current.parse(JSON.parse(card));
    } else {
      cardRef.current = card;
    }

    cardRef.current.onExecuteAction = (action) => {
      if (action instanceof AC.SubmitAction && action.id) {
        if (!submittingRef.current) {
          submittingRef.current = true;

          const data: Record<string, any> = {
            ...(action.data || null),
            submitId: action.id,
          };

          // hack
          if (data.action === 'cancel') {
            onCancel();
          }

          mpMenu.submitAdaptiveCardResponse(data);
        }
      } else if (action instanceof AC.OpenUrlAction && action.url) {
        mpMenu.openUrl(action.url);
      }
    };

    submittingRef.current = false;

    renderCard();
  }, [card, onCancel]);

  return (
    <>
      <Scrollable>
        <div ref={containerRef} className={s.root} />
      </Scrollable>

      <Pad size="small">
        <Text asDiv size="xsmall" opacity="50" centered>
          Disclaimer: UI above has been provided by the server and is <strong>not part of the {CurrentGameBrand} UI</strong>
        </Text>
      </Pad>
    </>
  );
});

function getHostConfig(uiService: IUiService): AC.HostConfig {
  function fontSize(size: number) {
    return (uiService.quant * 2.5) * size;
  }

  function q(m: number): number {
    return uiService.quant * m;
  }

  const fontSizes = {
    small: fontSize(.8),
    default: fontSize(1),
    medium: fontSize(1.25),
    large: fontSize(1.5),
    extraLarge: fontSize(1.75),
  };

  const fontWeights = {
    lighter: 200,
    default: 400,
    bolder: 500,
  };

  /**
   * @include ui.def('offset-xsmall', ui.q(.5));
   * @include ui.def('offset-small', ui.q(1));
   * @include ui.def('offset-normal', ui.q(2));
   * @include ui.def('offset-large', ui.q(4));
   * @include ui.def('offset-xlarge', ui.q(6));
   */
  const spacing = {
    small: q(1),
    default: q(2),
    medium: q(4),
    large: q(6),
    extraLarge: q(8),

    padding: q(4),
  };

  return new AC.HostConfig({
    supportsInteractivity: true,

    spacing,
    actions: {
      actionsOrientation: 'horizontal',
      actionAlignment: AC.ActionAlignment.Left,
    },
    fontTypes: {
      default: {
        fontFamily: 'RubikVariable',
        fontSizes,
        fontWeights,
      },

      monospace: {
        fontSizes,
        fontWeights,
      }
    },
    // imageSizes: {
    //   small: 40,
    //   medium: 80,
    //   large: 120
    // },
    // imageSet: {
    //   imageSize: 'medium',
    //   maxImageHeight: 100
    // },
    separator: {
      lineColor: 'rgba(var(--color-main-100), 1.0)',
    },
    containerStyles: {
      default: {
        foregroundColors: {
          default: {
            default: 'var(--color-text)',
            subtle: '#88FFFFFF',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          dark: {
            default: '#000000',
            subtle: '#66000000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          light: {
            default: '#FFFFFF',
            subtle: '#33000000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          accent: {
            default: 'rgba(var(--color-teal), 1.0)',
            subtle: 'rgba(var(--color-teal), .75)',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          good: {
            default: '#00FF00',
            subtle: '#DD00FF00',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          warning: {
            default: '#FFD800',
            subtle: '#DDFFD800',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          attention: {
            default: '#FF0000',
            subtle: '#DDFF0000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          }
        },
        backgroundColor: 'var(--color-backdrop)'
      },
      emphasis: {
        foregroundColors: {
          default: {
            default: 'var(--color-text)',
            subtle: '#88FFFFFF',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          dark: {
            default: '#000000',
            subtle: '#66000000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          light: {
            default: '#FFFFFF',
            subtle: '#33000000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          accent: {
            default: '#2E89FC',
            subtle: '#882E89FC',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          good: {
            default: '#00FF00',
            subtle: '#DD00FF00',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          warning: {
            default: '#FF0000',
            subtle: '#DDFF0000',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          },
          attention: {
            default: '#FFD800',
            subtle: '#DDFFD800',
            highlightColors: {
              default: '#22000000',
              subtle: '#11000000'
            }
          }
        },
        backgroundColor: 'rgba(var(--color-main-100), 1)'
      }
    }
  });
}
