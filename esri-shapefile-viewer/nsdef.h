#ifndef NSDEF_H
#define NSDEF_H

namespace cl
{
template<typename T>
class Pair;
class Bounds;

namespace Dataset
{
class ShapeDatasetPtr;
class ShapeDatasetRC;
class ShapeRecordUnique;
}

namespace Graphics
{
class Shape;
class ShapePrivate;
class Point;
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
