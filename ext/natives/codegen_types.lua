type 'Void'
	nativeType 'int'

type 'AnyPtr'
	nativeType 'Any*'

type 'Any'
	nativeType 'int'

type 'uint'
	nativeType 'int'

type 'Hash'
	nativeType 'int'

type 'Entity'
	nativeType 'int'

type 'Player'
	nativeType 'int'

type 'FireId'
	nativeType 'int'

type 'Ped'
	nativeType 'int'
	extends 'Entity'

type 'Vehicle'
	nativeType 'int'
	extends 'Entity'

type 'Cam'
	nativeType 'int'

type 'CarGenerator'
	nativeType 'int'

type 'Group'
	nativeType 'int'

type 'Train'
	nativeType 'int'
	extends 'Vehicle'

type 'Pickup'
	nativeType 'int'

type 'Object'
	nativeType 'int'
	extends 'Entity'

type 'Weapon'
	nativeType 'int'

type 'Interior'
	nativeType 'int'

type 'Blip'
	nativeType 'int'

type 'Texture'
	nativeType 'int'

type 'TextureDict'
	nativeType 'int'

type 'CoverPoint'
	nativeType 'int'

type 'Camera'
	nativeType 'int'

type 'TaskSequence'
	nativeType 'int'

type 'ColourIndex'
	nativeType 'int'

type 'Sphere'
	nativeType 'int'

type 'ScrHandle'
	nativeType 'int'

type 'BOOL'
	nativeType 'bool'

-- you wouldn't say
type 'int'
	nativeType 'int'

type 'long'
	nativeType 'int'
	subType 'long'

type 'float'
	nativeType 'float'

type 'charPtr'
	nativeType 'string'

type 'Vector3'
	nativeType 'vector3'

type 'func'
	nativeType 'func'

type 'object'
	nativeType 'object'