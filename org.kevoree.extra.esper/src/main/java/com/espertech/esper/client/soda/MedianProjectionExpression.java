/**************************************************************************************
 * Copyright (C) 2008 EsperTech, Inc. All rights reserved.                            *
 * http://esper.codehaus.org                                                          *
 * http://www.espertech.com                                                           *
 * ---------------------------------------------------------------------------------- *
 * The software in this package is published under the terms of the GPL license       *
 * a copy of which has been included with this distribution in the license.txt file.  *
 **************************************************************************************/
package com.espertech.esper.client.soda;

import java.io.StringWriter;

/**
 * Median projection (aggregation) in the distinct and regular form.
 */
public class MedianProjectionExpression extends ExpressionBase
{
    private boolean distinct;
    private static final long serialVersionUID = 4859634868887801690L;

    /**
     * Ctor.
     */
    public MedianProjectionExpression() {
    }

    /**

     * Ctor - for use to create an expression tree, without inner expression
     * @param isDistinct true if distinct
     */
    public MedianProjectionExpression(boolean isDistinct)
    {
        this.distinct = isDistinct;
    }

    /**
     * Ctor - adds the expression to project.
     * @param expression returning values to project
     * @param isDistinct true if distinct
     */
    public MedianProjectionExpression(Expression expression, boolean isDistinct)
    {
        this.distinct = isDistinct;
        this.getChildren().add(expression);
    }

    public ExpressionPrecedenceEnum getPrecedence()
    {
        return ExpressionPrecedenceEnum.UNARY;
    }

    public void toPrecedenceFreeEPL(StringWriter writer)
    {
        writer.write("median(");
        if (distinct)
        {
            writer.write("distinct ");
        }
        this.getChildren().get(0).toEPL(writer, ExpressionPrecedenceEnum.MINIMUM);
        writer.write(")");
    }

    /**
     * Returns true if the projection considers distinct values only.
     * @return true if distinct
     */
    public boolean isDistinct()
    {
        return distinct;
    }

    /**
     * Returns true if the projection considers distinct values only.
     * @return true if distinct
     */
    public boolean getDistinct()
    {
        return distinct;
    }

    /**
     * Set the distinct flag indicating the projection considers distinct values only.
     * @param distinct true for distinct, false for not distinct
     */
    public void setDistinct(boolean distinct)
    {
        this.distinct = distinct;
    }
}
