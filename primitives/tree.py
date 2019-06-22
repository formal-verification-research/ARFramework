class Tree:
    class Node:
        def __init__(self, val):
            self.l = None
            self.r = None
            self.p = None
            self.v = val
            self.__red = True

        def gp():
            def fget(self):
                if self.p is None: return None
                return self.p.p
            return locals()
        gp = property(**gp())

        def red():
            def fget(self):
                return self.__red
            def fset(self, value):
                self.__red = value
            return locals()
        red = property(**red())

        def black():
            def fget(self):
                return not self.__red
            def fset(self, value):
                self.__red = not value
            return locals()
        black = property(**black())

        def __repr__(self):
            return repr(self.v)

    def __init__(self, compFunc, initialValues=[]):
        self._root = None
        self.compFunc = compFunc
        self.size = 0
        for val in initialValues:
            self.insert(val)

    def getUncle(self, node):
        if node.p is None or node.gp is None: return None
        comp = self.compFunc(node.p.v, node.gp.v)
        if comp == -1: return node.gp.r
        if comp == 1: return node.gp.l
        assert False, "Tree is broken!"

    def insert(self, val):
        if self._root == None :
            self._root = Tree.Node(val)
            self._root.black = True
        else:
            self.__insert(val, self._root)

    def __insert(self, val, node):
        comp = self.compFunc(val, node.v)
        if comp == -1:
            if node.l is not None:
                self.__insert(val, node.l)
            else:
                node.l = Tree.Node(val)
                node.l.p = node
                self.size += 1
        elif comp == 1:
            if node.r is not None:
                self.__insert(val, node.r)
            else:
                node.r = Tree.Node(val)
                node.r.p = node
                self.size += 1

    def find(self, val):
        return self.__find(val, self._root)

    def __find(self, val, node):
        if node is None:
            return None
        comp = self.compFunc(val, node.v)
        if(comp == 0):
            return node
        elif comp == -1:
            return self.__find(val, node.l)
        elif comp == 1:
            return self.__find(val, node.r)

    def pop_root(self):
        if self._root is None:
            return None
        oldRoot = self._root
        if self._root.l is None and self._root.r is None:
            self._root = None
        elif self._root.l is None or self._root.r is None:
            if self._root.l is not None:
                self._root = oldRoot.l
                oldRoot.l = None
            else:
                self._root = oldRoot.r
                oldRoot.r = None
            self._root.p = None
        else:
            nextRoot = self._root.l
            if nextRoot.r is None:
                self._root = nextRoot
                self._root.r = oldRoot.r
                oldRoot.l = None
                oldRoot.r = None
            else:
                parent = self._root
                while nextRoot.r is not None:
                    parent = nextRoot
                    nextRoot = nextRoot.r
                parent.r = nextRoot.l
                nextRoot.l = None
                self._root = nextRoot
                self._root.l = oldRoot.l
                self._root.r = oldRoot.r
                oldRoot.l = None
                oldRoot.r = None
            if nextRoot.l is not None: nextRoot.l.p = nextRoot
            if nextRoot.r is not None: nextRoot.r.p = nextRoot
            nextRoot.p = None
        self.size -= 1
        return oldRoot.v

    def remove(self, val):
        if self._root is None:
            return None
        comp = self.compFunc(val, self._root.v)
        if comp == 0:
            return self.pop_root()
        if comp == -1:
            return self.__removeLeft(val, self._root)
        return self.__removeRight(val, self._root)

    def __removeLeft(self, val, node):
        if node.l is None:
            return None
        comp = self.compFunc(val, node.l.v)
        if comp == 0:
            toRemove = node.l
            if toRemove.l is None and toRemove.r is None:
                node.l = None
            elif toRemove.l is None or toRemove.r is None:
                if toRemove.l is not None:
                    node.l = toRemove.l
                    toRemove.l = None
                else:
                    node.l = toRemove.r
                    toRemove.r = None
                node.l.p = node
            else:
                nextRoot = toRemove.l
                if nextRoot.r is None:
                    node.l = nextRoot
                    node.l.r = toRemove.r
                    toRemove.l = None
                    toRemove.r = None
                else:
                    parent = toRemove
                    while nextRoot.r is not None:
                        parent = nextRoot
                        nextRoot = nextRoot.r
                    parent.r = nextRoot.l
                    nextRoot.l = None
                    node.l = nextRoot
                    node.l.l = toRemove.l
                    node.l.r = toRemove.r
                    toRemove.l = None
                    toRemove.r = None
                if nextRoot.l is not None: nextRoot.l.p = nextRoot
                if nextRoot.r is not None: nextRoot.r.p = nextRoot
                nextRoot.p = node
            self.size -= 1
            return toRemove.v
        if comp == -1:
            return self.__removeLeft(val, node.l)
        return self.__removeRight(val, node.l)

    def __removeRight(self, val, node):
        if node.r is None:
            return None
        comp = self.compFunc(val, node.r.v)
        if comp == 0:
            toRemove = node.r
            if toRemove.l is None and toRemove.r is None:
                node.r = None
            elif toRemove.l is None or toRemove.r is None:
                if toRemove.l is not None:
                    node.r = toRemove.l
                    toRemove.l = None
                else:
                    node.r = toRemove.r
                    toRemove.r = None
                node.r.p = node
            else:
                nextRoot = toRemove.l
                if nextRoot.r is None:
                    node.r = nextRoot
                    node.r.r = toRemove.r
                    toRemove.l = None
                    toRemove.r = None
                else:
                    parent = toRemove
                    while nextRoot.r is not None:
                        parent = nextRoot
                        nextRoot = nextRoot.r
                    parent.r = nextRoot.l
                    nextRoot.l = None
                    node.r = nextRoot
                    node.r.l = toRemove.l
                    node.r.r = toRemove.r
                    toRemove.l = None
                    toRemove.r = None
                if nextRoot.l is not None: nextRoot.l.p = nextRoot
                if nextRoot.r is not None: nextRoot.r.p = nextRoot
                nextRoot.p = node
            self.size -= 1
            return toRemove.v
        if comp == -1:
            return self.__removeLeft(val, node.r)
        return self.__removeRight(val, node.r)

    def traversal(self, func):
        self.__inorder_traversal(self._root, func)

    def __inorder_traversal(self, node, func):
        if node is None:
            return
        self.__inorder_traversal(node.l, func)
        func(node.v)
        self.__inorder_traversal(node.r, func)

    def inorder_generator(self):
        yield from self.__inorder_generator(self._root)

    def __inorder_generator(self, node):
        if node is not None:
            yield from self.__inorder_generator(node.l)
            yield node.v
            yield from self.__inorder_generator(node.r)

    def preorder_generator(self):
        yield from self.__preorder_generator(self._root)

    def __preorder_generator(self, node):
        if node is not None:
            yield node.v
            yield from self.__preorder_generator(node.l)
            yield from self.__preorder_generator(node.r)

    def postorder_generator(self):
        yield from self.__postorder_generator(self._root)

    def __postorder_generator(self, node):
        if node is not None:
            yield from self.__postorder_generator(node.l)
            yield from self.__postorder_generator(node.r)
            yield node.v

    def __iter__(self, traversal='preorder'):
        if traversal == 'inorder':
            yield from self.inorder_generator()
        elif traversal == 'preorder':
            yield from self.preorder_generator()
        elif traversal == 'postorder':
            yield from self.postorder_generator()

    def __str__(self):
        return ' '.join(str(x) for x in self.__iter__(traversal='inorder'))

    def __repr__(self):
        return str(list(self.__iter__(traversal='preorder')))

    def __len__(self):
        return sum(1 for i in iter(self))

    def __rightRotateRoot(self):
        if self._root is None or self._root.l is None: return
        oldRoot = self._root
        self._root = oldRoot.l
        oldRoot.l = self._root.r
        self._root.r = oldRoot
        if self._root.r is not None:
            self._root.r.p = self._root
            if self._root.r.l is not None:
                self._root.r.l.p = self._root.r
        if self._root.l is not None: self._root.l.p = self._root
        self._root.p = None

    def _rightRotate(self, node, right=True):
        if node is None: return
        if right is None and node is not self._root:
            assert False, "Cannot perform rotation on node passed by value"
        if node is self._root and right is None:
            self.__rightRotateRoot()
            return
        if right:
            if node.r is None or node.r.l is None: return
            n = node.r
            node.r = n.l
            n.l = node.r.r
            node.r.r = n
            if node.r.r is not None:
                node.r.r.p = node.r
                if node.r.r.l is not None:
                    node.r.r.l.p = node.r.r
            if node.r.l is not None: node.r.l.p = node.r
            node.r.p = node
        else:
            if node.l is None or node.l.l is None: return
            n = node.l
            node.l = n.l
            n.l = node.l.r
            node.l.r = n
            if node.l.r is not None:
                node.l.r.p = node.l
                if node.l.r.l is not None:
                    node.l.r.l.p = node.l.r
            if node.l.l is not None: node.l.l.p = node.l
            node.l.p = node

    def __leftRotateRoot(self):
        if self._root is None or self._root.r is None: return
        oldRoot = self._root
        self._root = oldRoot.r
        oldRoot.r = self._root.l
        self._root.l = oldRoot
        if self._root.l is not None:
            self._root.l.p = self._root
            if self._root.l.r is not None:
                self._root.l.r.p = self._root.l
        if self._root.r is not None: self._root.r.p = self._root
        self._root.p = None

    def _leftRotate(self, node, right=True):
        if node is None: return
        if right is None and node is not self._root:
            assert False, "Cannot perform rotation on node passed by value"
        if node is self._root and right is None:
            self.__leftRotateRoot()
            return
        if right:
            if node.r is None or node.r.r is None: return
            n = node.r
            node.r = n.r
            n.r = node.r.l
            node.r.l = n
            if node.r.l is not None:
                node.r.l.p = node.r
                if node.r.l.r is not None:
                    node.r.l.r.p = node.r.l
            if node.r.r is not None: node.r.r.p = node.r
            node.r.p = node
        else:
            if node.l is None or node.l.r is None: return
            n = node.l
            node.l = n.r
            n.r = node.l.l
            node.l.l = n
            if node.l.l is not None:
                node.l.l.p = node.l
                if node.l.l.r is not None:
                    node.l.l.r.p = node.l.l
            if node.l.r is not None: node.l.r.p = node.l
            node.l.p = node


def compFuncInt(a,b):
    if a < b: return -1
    elif a > b: return 1
    return 0

tree = Tree(compFuncInt)
tree.insert(5)
tree.insert(6)
tree.insert(4)
tree.insert(8)
tree.insert(7)
tree.insert(2)
tree.insert(3)
tree.insert(19)
tree.insert(12)
tree.insert(11)

print(str(tree))
