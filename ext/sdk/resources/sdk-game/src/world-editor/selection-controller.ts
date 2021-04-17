import { sendSdkMessage } from "../client/sendSdkMessage";
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

    const [f, r, u, a] = GetEntityMatrix(this.selectedEntity);
    const data = new Float32Array([
      r[0], r[1], r[2], 0,
      f[0], f[1], f[2], 0,
      u[0], u[1], u[2], 0,
      a[0], a[1], a[2], 1,
    ]);

    if (DrawGizmo(data as any, this.selectedEntity.toString())) {
      SetEntityMatrix(
        this.selectedEntity,
        data[4], data[5], data[6], // r
        data[0], data[1], data[2], // f
        data[8], data[9], data[10], // u
        data[12], data[13], data[14], // a
      );
    }
  }
};

const select = useKeyMapping('we_select', 'Select entity', 'mouse_button', 'mouse_left');
select.on(() => SelectionController.select(true));
select.off(() => SelectionController.select(false));
