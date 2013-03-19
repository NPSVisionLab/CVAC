import cvac

class Corpus:
    """A corpus or set of media data, often labeled"""

    def hasCategories(self):
        return len(self.categories)>0
    def __init__(self, name):
        self.name = cvac.corpus(name)
        self.categories = ['horses', 'cats', 'houses']
