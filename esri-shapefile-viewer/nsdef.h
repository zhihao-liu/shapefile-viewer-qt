#ifndef NSDEF_H
#define NSDEF_H

namespace cl
{
template<typename T>
class Pair;
class Bounds;
class Drawing;

namespace Dataset
{
class ShapeDatasetShared;
class ShapeRecordUnique;
}

namespace Graphics
{
class Shape;
class ShapePrivate;

class Point;
class MultiPartShape;
class Polyline;
class Polygon;

class GraphicAssistant;
class GraphicAssistantPrivate;
}

namespace DataManagement
{
class ShapeFactory;
class ShapeFactoryESRI;

class ShapeManager;
class ShapeDocs;
class ShapeDocsPrivate;
class ShapeDocsObserver;
}
}
#endif // NSDEF_H
