#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

BoxPathPoint::BoxPathPoint(QPointFAnimator *associatedAnimator,
                           BoxTransformAnimator *box) :
    AnimatedPoint(associatedAnimator, box, TYPE_PIVOT_POINT, 7.) {}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    mSavedAbsPos = getAbsolutePos();
    const auto boxTrans = GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    boxTrans->startPivotTransform();
}

void BoxPathPoint::finishTransform() {
    MovablePoint::finishTransform();
    const auto boxTrans = GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    boxTrans->finishPivotTransform();
}

void BoxPathPoint::moveByAbs(const QPointF &absTranslatione) {
    const QPointF absPos = mSavedAbsPos + absTranslatione;
    const auto boxTrans = GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    const auto parentBox = boxTrans->getParentBox();
    parentBox->setPivotAutoAdjust(false);
    parentBox->setPivotAbsPos(absPos);
}

void BoxPathPoint::drawSk(SkCanvas * const canvas,
                          const SkScalar &invScale) {
    if(isHidden()) return;
    const SkPoint absPos = toSkPoint(getAbsolutePos());
    const SkColor fillCol = mSelected ?
                SkColorSetRGB(255, 255, 0) :
                SkColorSetRGB(255, 255, 125);
    drawOnAbsPosSk(canvas, absPos, invScale, fillCol);

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    const SkScalar scaledHalfRadius = toSkScalar(mRadius)*invScale*0.5f;
    canvas->drawLine(-scaledHalfRadius, 0, scaledHalfRadius, 0, paint);
    canvas->drawLine(0, -scaledHalfRadius, 0, scaledHalfRadius, paint);
    canvas->restore();
}
