const LOAD_MODEL = -2;
const MODEL_LOADING = -1;

const objects = {};

setTick(() => {
  const [x, y] = GetEntityCoords(PlayerPedId());
  const objectIndices = mai.search(x-50, y-50, x+50, y+50).map(String);

  for (const i of objectIndices) {
    if (i in objects === false) {
      objects[i] = LOAD_MODEL;
    }
  }

  for (const i in objects) {
    const handle = objects[i];
    const data = mad[i];

    if (objectIndices.indexOf(i) === -1) {
      if (handle > MODEL_LOADING) {
        DeleteObject(handle);
      }

      delete objects[i];
    } else if (handle === LOAD_MODEL) {
      RequestModel(data[0]);
      objects[i] = MODEL_LOADING;
    } else if (handle === MODEL_LOADING) {
      if (HasModelLoaded(data[0])) {
        const mat = data[1];

        objects[i] = CreateObject(data[0], mat[12], mat[13], mat[14], false, false, false);
        SetEntityMatrix(
          objects[i],
          mat[4], mat[5], mat[6], // right
          mat[0], mat[1], mat[2], // forward
          mat[8], mat[9], mat[10], // up
          mat[12], mat[13], mat[14], // at
        );
      }
    }
  }
});
