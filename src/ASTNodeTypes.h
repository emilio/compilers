/*
 * Copyright (C) 2017 Emilio Cobos Álvarez <emilio@crisal.io>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** Helper file just to use some preprocessor magic to avoid duplication */

NODE_TYPE(Expression)
NODE_TYPE(ConstantExpression)
NODE_TYPE(UnaryOperation)
NODE_TYPE(BinaryOperation)
NODE_TYPE(FunctionCall)
NODE_TYPE(ParenthesizedExpression)
NODE_TYPE(VariableBinding)
NODE_TYPE(Block)
NODE_TYPE(Statement)
NODE_TYPE(ConditionalExpression)
NODE_TYPE(ForLoop)
