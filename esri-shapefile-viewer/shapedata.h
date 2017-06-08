#ifndef SHAPEDATA_H
#define SHAPEDATA_H

#include <string>
#include <memory>
#include <vector>
#include "../shapelib/shapefil.h"
#include "nsdef.h"
#include "support.h"

class QPainter;
class QPoint;
class QColor;

enum class cl::Dataset::ShapeType
{
    Unknown = 0,
    Point,
    Polyline,
    Polygon
};

class cl::Dataset::ShapeRecordUnique
{
public:
    ~ShapeRecordUnique();

    ShapeRecordUnique() : _raw(nullptr) {}
    ShapeRecordUnique(ShapeDatasetShared const& ptrDataset, int index);

    ShapeRecordUnique(ShapeRecordUnique const& rhs) = delete;
    ShapeRecordUnique& operator= (ShapeRecordUnique const& rhs) = delete;

    // The move constructor and the move assignment operator,
    // used for passing an r-value.
    ShapeRecordUnique(ShapeRecordUnique&& rhs);
    ShapeRecordUnique& operator= (ShapeRecordUnique&& rhs);

    SHPObject& operator* () { return *_raw; }
    SHPObject* operator-> () { return _raw; }

    bool operator== (void* other) const { return _raw == other ? true : false; }
    bool operator!= (void* other) const { return _raw != other ? true : false; }

private:
    SHPObject* _raw;
};

class cl::Dataset::ShapeDatasetShared
{
private:
    class RC;
    RC* _raw;

public:
    ShapeDatasetShared() : _raw(nullptr) {}
    ShapeDatasetShared(std::string const& path);
    ShapeDatasetShared(RC* shapeDataset);

    ShapeDatasetShared(ShapeDatasetShared const& rhs);
    ShapeDatasetShared& operator= (ShapeDatasetShared const& rhs);

    ~ShapeDatasetShared();

    RC& operator* () const { return *_raw; }
    RC* operator-> () const { return _raw; }

    bool operator== (void* that) const { return _raw == that ? true : false; }
    bool operator!= (void* that) const { return _raw != that ? true : false; }

    ShapeRecordUnique readRecord(int index) const;
};

class cl::Dataset::ShapeDatasetShared::RC
{
    friend class ShapeDatasetShared;

public:
    ~RC();

    ShapeType type() const { return _type; }
    SHPHandle const& handle() const { return _shpHandle; }
    SHPTree const& tree() const { return *_shpTree; }
    int recordCount() const { return _shpHandle->nRecords;}
    Rect<double> const& bounds() const { return _bounds; }
    std::string const& name() const { return _name; }
    std::vector<int> const filterRecords(Rect<double> const& mapHitBounds) const;

private:
    RC(std::string const& path);

    SHPInfo* _shpHandle;
    SHPTree* _shpTree;
    ShapeType _type;
    std::string _name;
    Rect<double> _bounds;
    int _refCount;

    RC* addRef();
};

class cl::Graphics::Shape
{
public:
    virtual ~Shape();

    std::shared_ptr<Shape> clone() const;
    std::string const& name() const;
    int recordCount() const;
    Rect<double> const& bounds() const;

    // Return the number of records hit according to the index tree.
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const = 0;

protected:
    Shape(Dataset::ShapeDatasetShared const& ptrDataset);

    class Private;
    std::unique_ptr<Private> _private;
};

class cl::Graphics::Point : public Shape
{
public:
    Point(Dataset::ShapeDatasetShared ptrDataset): Shape(ptrDataset) {}
    virtual ~Point() {}
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const override;
};

class cl::Graphics::MultiPartShape : public Shape
{
public:
    MultiPartShape(Dataset::ShapeDatasetShared ptrDataset): Shape(ptrDataset) {}
    virtual ~MultiPartShape() {}

protected:
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const override;
    virtual void drawPart(QPainter& painter, QPoint const* points, int pointCount) const = 0;
};

class cl::Graphics::Polyline : public MultiPartShape
{
public:
    Polyline(Dataset::ShapeDatasetShared ptrDataset): MultiPartShape(ptrDataset) {}
    virtual ~Polyline() {}
    virtual void drawPart(QPainter& painter, QPoint const* points, int pointCount) const override;
};

class cl::Graphics::Polygon : public MultiPartShape
{
public:
    Polygon(Dataset::ShapeDatasetShared ptrDataset): MultiPartShape(ptrDataset) {}
    virtual ~Polygon() {}
    virtual void drawPart(QPainter& painter, QPoint const* points, int pointCount) const override;
};

enum class cl::DataManagement::ShapeProvider { ESRI = 0, AUTODESK, OTHERS };

class cl::DataManagement::ShapeFactory
{
public:
    virtual ~ShapeFactory() = default;
    virtual std::shared_ptr<Graphics::Shape> createShape(std::string const& path) const = 0;

protected:
    ShapeFactory() = default;
};

class cl::DataManagement::ShapeFactoryEsri: ShapeFactory
{
public:
    virtual ~ShapeFactoryEsri() = default;
    virtual std::shared_ptr<Graphics::Shape> createShape(std::string const& path) const override;
    static ShapeFactory const& instance();

private:
    ShapeFactoryEsri() = default;
    static std::unique_ptr<ShapeFactory> _instance;
};

#endif // SHAPEDATA_H
