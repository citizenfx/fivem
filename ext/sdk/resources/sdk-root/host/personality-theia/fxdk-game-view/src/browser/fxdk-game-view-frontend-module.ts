import { FxdkGameView, FxdkGameViewContribution } from './fxdk-game-view-view';
import { ContainerModule } from "inversify";
import { bindViewContribution, WidgetFactory } from '@theia/core/lib/browser';

export default new ContainerModule(bind => {
  bindViewContribution(bind, FxdkGameViewContribution);
  bind(FxdkGameView).toSelf();
  bind(WidgetFactory).toDynamicValue(ctx => ({
      id: FxdkGameView.ID,
      createWidget: () => ctx.container.get<FxdkGameView>(FxdkGameView)
  }));
});
