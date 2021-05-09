import { sendSdkMessage } from "../client/sendSdkMessage";
import { applyEntityMatrix, makeEntityMatrix } from "./math";
import { useKeyMapping } from "./utils";

export const SelectionController = new class SelectionController {
  private selectStart: number = 0;
  private selectedEntity: number | null = null;

  select(active: boolean) {
    if (active) {
      this.selectStart = GetGameTimer();
      return;
    }

    if (GetGameTimer() - this.selectStart > 250) {
      console.log('No select as >250ms hold');
      return;
    }

    const selectedEntity = SelectEntityAtCursor(6 | (1 << 5), true);
    if (selectedEntity) {
      if (this.selectedEntity !== null) {
        SetEntityDrawOutline(this.selectedEntity, false);
        ReleaseScriptGuidFromEntity(this.selectedEntity);


        sendSdkMessage('we:selection', null);
      }

      if (this.selectedEntity !== selectedEntity) {
        this.selectedEntity = selectedEntity;
        SetEntityDrawOutline(this.selectedEntity, true);

        sendSdkMessage('we:selection', {
          id: this.selectedEntity,
          lod: GetEntityLodDist(this.selectedEntity),
          type: GetEntityType(this.selectedEntity),
          model: GetEntityModel(this.selectedEntity),
        });
      }
    }
  }

  update() {
    if (this.selectedEntity === null) {
      return;
    }

    const data = makeEntityMatrix(this.selectedEntity);

    if (DrawGizmo(data as any, this.selectedEntity.toString())) {
      applyEntityMatrix(this.selectedEntity, data);
    }
  }
};

const select = useKeyMapping('we_select', 'Select entity', 'mouse_button', 'mouse_left');
select.on(() => SelectionController.select(true));
select.off(() => SelectionController.select(false));
